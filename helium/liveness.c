#include <string.h>

#include "ext/bitarray.h"
#include "ext/table.h"
#include "ext/mem.h"
#include "ext/log.h"

#include "graph.h"
#include "liveness.h"
#include "flowgraph.h"
#include "asm.h"

typedef struct
{
    BitArray in;
    BitArray out;
} * Live_info;

static BitArray Use (G_node node, Flow_graph flow)
{
    return MapTempList (flow->tempsNum, FG_Use (node), flow->temps);
}

static BitArray Def (G_node node, Flow_graph flow)
{
    return MapTempList (flow->tempsNum, FG_Def (node), flow->temps);
}

Live_moveList Live_MoveList (ASM_line line, G_node src, G_node dst, Live_moveList tail)
{
    Live_moveList r = checked_malloc (sizeof (*r));
    r->line = line;
    r->src = src;
    r->dst = dst;
    r->tail = tail;
    return r;
}

static void Liveness (Flow_graph flow, TAB_table nodes)
{
    /* DBG("Liveness:\n"); */
    /*
     * Get reversed node list so it will take less iterations to
     * calculate in/out info
     */
    G_nodeList list = NULL;
    for (G_nodeList l = G_Nodes (flow->graph); l; l = l->tail)
    {
        list = G_NodeList (l->head, list);
    }

    BitArray in = BitArray_New (flow->tempsNum, FALSE);
    BitArray out = BitArray_New (flow->tempsNum, FALSE);
    bool changed = TRUE;
    while (changed)
    {
        /* DBG("interation %d\n", ++inter); */
        changed = FALSE;
        for (G_nodeList l = list; l; l = l->tail)
        {
            G_node node = l->head;
            Live_info li = TAB_Look (nodes, node);

            BitArray_Clone (li->in, in);
            BitArray_Clone (li->out, out);

            // in[n] = use[n] ∪ ( out[n] ∖ def[n] )
            BitArray_UnSetAll (li->in); // TODO: check this
            BitArray_RelDifference (li->out, Def (node, flow), li->in);
            BitArray_Union (Use (node, flow), li->in, li->in);

            // out[n] = U(s ∈ succ[n]) in[s]
            BitArray_UnSetAll (li->out); // TODO: check this
            for (G_nodeList sn = G_Succ (node); sn; sn = sn->tail)
            {
                Live_info sni = TAB_Look (nodes, sn->head);
                BitArray_Union (li->out, sni->in, li->out);
            }

            if (!BitArray_Equal (in, li->in) || !BitArray_Equal (out, li->out))
            {
                changed = TRUE;
            }
            /* ASM_line i = G_NodeInfo(node); */
            /* Temp_map m = Temp_Name(); */
            /* char r[200]; #<{(| result |)}># */
            /* switch (i->kind) */
            /* { */
            /* case I_LABEL: */
            /*     ASM_Format (r, i->u.LABEL.assem, NULL, NULL, NULL, m); */
            /*     fprintf (stdout, "%-20s", r); */
            /*     #<{(| i->u.LABEL->label); |)}># */
            /*     break; */
            /* case I_OPER: */
            /*     ASM_Format (r, i->u.OPER.assem, i->u.OPER.dst, i->u.OPER.src, i->u.OPER.jumps, m); */
            /*     fprintf (stdout, "  %-20s", r); */
            /*     break; */
            /* case I_MOVE: */
            /*     ASM_Format (r, i->u.MOVE.assem, i->u.MOVE.dst, i->u.MOVE.src, NULL, m); */
            /*     fprintf (stdout, "  %-20s", r); */
            /*     break; */
            /* } */
            /* DBG(" in: %s", BitArray_ToString(li->in)) */
            /* DBG(" out: %s", BitArray_ToString(li->out)) */
            /* DBG("\n"); */
        }
        /* DBG("\n"); */
    }
}

static struct Live_graph Interference (Flow_graph flow, TAB_table nodes)
{
    G_graph g = G_Graph();
    TAB_table nd = TAB_Empty();

    Live_moveList ml = NULL, mt = NULL;

    for (Temp_tempList l = flow->temps; l; l = l->tail)
    {
        G_node node = G_Node (g, l->head);
        TAB_Enter (nd, l->head, node);
    }

    int movesNum = 0;
    for (G_nodeList l = G_Nodes (flow->graph); l; l = l->tail)
    {
        Live_info li = TAB_Look (nodes, l->head);
        Temp_tempList def = FG_Def (l->head);
        Temp_tempList use = FG_Use (l->head);
        Temp_tempList out = UnmapTempList (li->out, flow->temps);

        if (def && def->head)
        {
            G_node dn = TAB_Look (nd, def->head);
            for (; out; out = out->tail)
            {
                G_node on = TAB_Look (nd, out->head);
                /*
                 * no self-edging
                 */
                if (dn != on)
                {
                    G_AddEdge (dn, on);
                }
            }
            /*
             * Move operators are treated specially, we do not create interference
             * between dst and src because the value is the same thus it will occupy
             * a single reg
             *
             * Here we just remove unneeded edge
             */
            ASM_line line = G_NodeInfo (l->head);
            if (line->kind == I_MOVE)
            {
                movesNum++;
                G_node un = TAB_Look (nd, use->head);
                /*
                 * no self-edging
                 */
                if (dn != un)
                {
                    G_RemoveEdge (dn, un);
                }
                if (mt)
                {
                    mt = mt->tail = Live_MoveList (line, dn, un, NULL);
                }
                else
                {
                    ml = mt = Live_MoveList (line, dn, un, NULL);
                }
            }
        }
    }

    struct Live_graph r;
    r.graph = g;
    r.moves = ml;
    r.movesNum = movesNum;
    return r;
}

struct Live_graph Live_Liveness (Flow_graph flow)
{
    /* map flow-graph-node to liveness-data */
    TAB_table nodes_t = TAB_Empty();

    for (G_nodeList l = G_Nodes (flow->graph); l; l = l->tail)
    {
        Live_info info = checked_malloc (sizeof (*info));
        info->in = BitArray_New (flow->tempsNum, FALSE);
        info->out = BitArray_New (flow->tempsNum, FALSE);
        TAB_Enter (nodes_t, l->head, info);
    }

    Liveness (flow, nodes_t);

    return Interference (flow, nodes_t);
}
