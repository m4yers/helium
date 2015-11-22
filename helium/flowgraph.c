#include <assert.h>

#include "ext/mem.h"
#include "ext/bool.h"
#include "ext/table.h"

#include "flowgraph.h"
#include "ast.h"

Temp_tempList FG_Def (G_node n)
{
    ASM_line l = (ASM_line) G_NodeInfo (n);
    return l->kind == I_OPER ? l->u.OPER.dst : l->u.MOVE.dst;
}

Temp_tempList FG_Use (G_node n)
{
    ASM_line l = (ASM_line) G_NodeInfo (n);
    return l->kind == I_OPER ? l->u.OPER.src : l->u.MOVE.src;
}

bool FG_IsMove (G_node n)
{
    return ((ASM_line)G_NodeInfo (n))->kind == I_MOVE;
}

Flow_graph FG_AsmFlowGraph (ASM_lineList l)
{
    G_graph g = G_Graph();
    G_node current = NULL, prev = NULL;

    /*
     * mnemonic: jump table, stores all possible instructions that can jump to a label
     */
    TAB_table tj = TAB_Empty();

    /*
     * mnemonic: destination table, stores the first instruction after a label
     */
    TAB_table td = TAB_Empty();

    /*
     * mnemonic: inctruction table, stores maps from instruction to its node.
     */
    TAB_table ti = TAB_Empty();

    /*
     * mnemonic: label list, stores all the labels you can jump to, used for tj and td lookup
     */
    Temp_labelList ll = NULL;

    Temp_tempList tl = Temp_TempList(NULL, NULL), tll = tl;
    int tlLen = 0;

    for (; l; l = l->tail)
    {
        ASM_line line = l->head;
        switch (line->kind)
        {
        case I_OPER:
        case I_MOVE:
        {
            if (line->kind == I_OPER && line->u.OPER.jumps)
            {
                for (Temp_labelList j = line->u.OPER.jumps->labels; j; j = j->tail)
                {
                    Temp_label label = j->head;
                    TAB_Enter (tj, label, ASM_LineList (line, TAB_Look (tj, label)));
                }
            }

            current = G_Node (g, line);
            if (prev)
            {
                G_AddEdge (prev, current);
            }
            TAB_Enter (ti, line, current);
            prev = current;

            // TODO: Artyom Goncharov Not very pretty - rewrite
            Temp_tempList dsts = FG_Def (current);
            for (; dsts; dsts = dsts->tail)
            {
                if (!Temp_IsTempInList (tl, dsts->head))
                {
                    tll = tll->tail = Temp_TempList (dsts->head, NULL);
                    tlLen++;
                }
            }
            Temp_tempList srcs = FG_Use (current);
            for (; srcs; srcs = srcs->tail)
            {
                if (!Temp_IsTempInList (tl, srcs->head))
                {
                    tll = tll->tail = Temp_TempList (srcs->head, NULL);
                    tlLen++;
                }
            }
            ////////////
            break;
        }
        case I_LABEL:
        {
            if (l->tail && l->tail->head->kind != I_LABEL)
            {
                Temp_label label = l->head->u.LABEL.label;
                ll = Temp_LabelList (label, ll);
                TAB_Enter (td, label, l->tail->head);
            }
            else // should not happen
            {
                // FIXME Artyom Goncharov (A) well, it does happen because of the munch func 
                /* assert (0); */
            }
            break;
        }
        }
    }

    /**
     * Adding edges between jumps and Follow(label)
     */
    for (; ll; ll = ll->tail)
    {
        Temp_label label = ll->head;
        ASM_lineList jl = TAB_Look (tj, label);
        ASM_line fol  = TAB_Look (td, label);

        // FIXME Artyom Goncharov (A) yeap 
        if (!jl || !fol)
        {
            continue;
        }

        assert (jl);
        assert (fol);

        G_node dst = TAB_Look (ti, fol);
        for (; jl; jl = jl->tail)
        {
            G_node src = TAB_Look (ti, jl->head);
            G_AddEdge (src, dst);
        }
    }

    Flow_graph r = checked_malloc (sizeof (*r));
    r->graph = g;
    r->temps = Temp_SortTempList(tl->tail);
    r->tempsNum = tlLen;

    return r;
}
