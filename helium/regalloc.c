/*
 *                    _,----,_                                _,----,
 *                 ,-'     /'          I           I          `\     `-,
 *               ,'       {,-~~-,     ;~;---------;~;     ,-~~-,}       `,
 *              /       _/',-~~-(\,)  | |  HERE   | |  (,/)-~~-,`\_       \
 *            ,'   ,-,/'{ (     {vv}  | |   BE    | |  {vv}     ) }`\,-,   `,
 *           , ,-,/      \ \     }{   | | DRAGONS | |   }{     / /      \,-, ,
 *           ;/           ) }   (^^)  ;_;---------;_;  (^^)   { (           \;
 * ,,,,,,_,-~~-,_,-~~-,_,/ /    )  (   I           I   )  (    \ \,_,-~~-,_,-~~-,_,,,,,,
 * ''''~-,,-~~-,_,-~~-,_,-'                                     `-,_,-~~-,_,-~~-,,-~````
 */
#include <assert.h>

#include "ext/bitarray.h"
#include "ext/table.h"
#include "ext/stack.h"
#include "ext/list.h"
#include "ext/util.h"
#include "ext/mem.h"

#define LOG_LEVEL LOG_LEVEL_WARNING
#include "ext/log.h"

#include "frame.h"
#include "regalloc.h"
#include "flowgraph.h"
#include "liveness.h"
#include "codegen.h"

typedef struct
{
    int index;
    BitArray edges;
} StackFrame;

typedef struct
{
    F_frame frame;
    Flow_graph flow;
    /**
     * Number of colors(registers) to use.
     */
    int K;
    /**
     * Registers available for colorizing
     */
    Temp_tempList regs;
    /**
     * List of assembly instructions to analyze
     */
    ASM_lineList asmll;
    /**
     * List of temporaries to be colorized. Among these temps real register
     * colors might occur(pre-colored temps)
     */
    Temp_tempList temps;
    /**
     * An interference graph produced by analyzing the assembly list
     */
    BitMatrix interference;
    /**
     * List of pre-colored temps
     */
    BitArray precolored;
    /**
     * List of temps to process
     */
    BitArray initial;
    /**
     * List of temps to be simplified
     */
    BitArray simplify;
    /*
     * Low-degree move-related nodes
     */
    BitArray freezify;
    /**
     * High-degree nodes
     */
    BitArray spillify;
    /**
     * List contains temps that are to be spilled for sure
     */
    BitArray spilled;
    /**
     */
    BitArray coalesced;
    int movesNum;
    /*
     * All move instructions in the fragment. The lists below are based on it.
     */
    Live_moveList moveList;
    /*
     * Stores mappings from a node to the list(mask) of move instructions it is
     * associated with.
     */
    TAB_table movesTable;
    /*
     * Moves that have been coalesced
     */
    BitArray coalescedMoves;
    /*
     * Moves whose source and target interfere
     */
    BitArray constrainedMoves;
    /*
     * Moves that no longer will be considered for coalescing
     */
    BitArray frozenMoves;
    /*
     * Moves enabled for possible coalescing
     */
    BitArray worklistMoves;
    /*
     * Moves  not yet ready for coalescing
     */
    BitArray activeMoves;
    /*
     * When a move (u, v)  has been coalesced, and v put in coalescedMoves, then
     * aliases[v] = u
     */
    TAB_table aliases;
    /*
     * Stores all access(stack frame space) used for new temps created during programm
     * rewrite
     */
    F_accessList accessList;
    /*
     * Look-up table for colors. By default it filled only with register temps that
     * mapped to themselves, so if some node already has a color it is already in
     * the table
     */
    TAB_table coloring;
    /*
     * List of colors used in coloring this code fragment
     */
    Temp_tempList colors;
    /*
     * The resulting temp-to-color map, this time it is string on the right side.
     */
    Temp_map results;
    /*
     * Stack stores simplified graph nodes
     */
    Stack stack;
} * Workspace;

// TODO: Artyom Goncharov TO BE REMOVED
static int GetIndex (Temp_temp t, Temp_tempList l)
{
    int i = 0;
    while (l)
    {
        if (t == l->head)
        {
            return i;
        }
        l = l->tail;
        i++;
    }

    return -1;
}

static Temp_temp GetTemp (int index, Temp_tempList l)
{
    assert (index >= 0);
    while (l)
    {
        if (index == 0)
        {
            return l->head;
        }
        l = l->tail;
        index--;
    }

    return NULL;
}

static int GetMoveIndex (ASM_line t, Live_moveList l)
{
    int i = 0;
    while (l)
    {
        if (t == l->line)
        {
            return i;
        }
        l = l->tail;
        i++;
    }

    return -1;
}

static ASM_line GetMove (int index, Live_moveList l)
{
    assert (index >= 0);
    while (l)
    {
        if (index == 0)
        {
            return l->line;
        }
        l = l->tail;
        index--;
    }

    return NULL;
}
//////////////////////

static Workspace
Workspace_New (F_frame f, ASM_lineList ll, F_registers regs_all, F_registers regs_colors, F_accessList accessList)
{
    DBG ("Workspace_New:\n");
    Flow_graph fg = FG_AsmFlowGraph (ll);

    DBG ("flow graph\n%s\n", G_ToString (G_Nodes (fg->graph)));

    struct Live_graph lg = Live_Liveness (fg);

    DBG ("\nliveness/interference graph\n%s\n", G_ToString (G_Nodes (lg.graph)));

    BitMatrix interference = BitMatrix_New (fg->tempsNum);

    G_nodeList nl = G_Nodes (lg.graph);
    while (nl)
    {
        int row = GetIndex (G_NodeInfo (nl->head), fg->temps);
        G_nodeList anl = G_Adj (nl->head);
        while (anl)
        {
            int col = GetIndex (G_NodeInfo (anl->head), fg->temps);
            BitMatrix_Set (interference, row, col);
            anl = anl->tail;
        }
        nl = nl->tail;
    }

    TAB_table coloring = TAB_Empty();
    LIST_FOREACH (r, regs_all->temps)
    {
        TAB_Enter (coloring, r, r);
        bool canUse = FALSE;

        /*
         * Registers that are present in regs_all but do not appear in regs_colors
         * considered special and MUST not be used in any way.
         */
        LIST_FOREACH (c, regs_colors->temps)
        {
            if (r == c)
            {
                canUse = TRUE;
                break;
            }
        }

        /*
         * Making it interfere with any other temp
         */
        if (!canUse)
        {
            int index = GetIndex (r, fg->temps);
            if (index >= 0)
            {
                BitMatrix_SetRow (interference, index);
                BitMatrix_SetColumn (interference, index);
            }
        }
    }

    int index = 0;
    BitArray precolored = BitArray_New (fg->tempsNum, FALSE);
    BitArray initial = BitArray_New (fg->tempsNum, FALSE);

    LIST_FOREACH (t, fg->temps)
    {
        bool match = FALSE;
        LIST_FOREACH (r, regs_all->temps)
        {
            if (r == t)
            {
                match = TRUE;
                break;
            }
        }

        if (match)
        {
            BitArray_Set (precolored, index);
        }
        else
        {
            BitArray_Set (initial, index);
        }

        index++;
    }

    Workspace r = checked_malloc (sizeof (*r));
    r->flow = fg;
    r->frame = f;
    r->K = regs_colors->number;
    r->regs = regs_colors->temps;
    r->asmll = ll;
    // sorting is unnecessary but helpful for debugging
    r->temps = Temp_SortTempList (fg->temps);
    r->interference = interference;

    r->precolored = precolored;
    r->initial = initial;
    r->simplify = BitArray_New (fg->tempsNum, FALSE);
    r->freezify = BitArray_New (fg->tempsNum, FALSE);
    r->spillify = BitArray_New (fg->tempsNum, FALSE);
    r->spilled = BitArray_New (fg->tempsNum, FALSE);
    r->coalesced = BitArray_New (fg->tempsNum, FALSE);

    if (lg.movesNum)
    {
        TAB_table movesTable = TAB_Empty();
        index = 0;
        for (Live_moveList l = lg.moves; l; l = l->tail)
        {
            // TODO: Artyom Goncharov nodes can come again so we need to read table first
            // at the momemnt only a single move node is valid
            Temp_temp dst = G_NodeInfo (l->dst);
            Temp_temp src = G_NodeInfo (l->src);
            BitArray a = BitArray_New (lg.movesNum, FALSE);
            BitArray_Set (a, index);
            TAB_Enter (movesTable, dst, a);
            TAB_Enter (movesTable, src, BitArray_Clone (a, NULL));
            index++;
        }

        r->movesNum = lg.movesNum;
        r->moveList = lg.moves;
        r->movesTable = movesTable;
        r->coalescedMoves = BitArray_New (lg.movesNum, FALSE);
        r->constrainedMoves = BitArray_New (lg.movesNum, FALSE);
        r->frozenMoves = BitArray_New (lg.movesNum, FALSE);
        r->worklistMoves = BitArray_New (lg.movesNum, TRUE);
        r->activeMoves = BitArray_New (lg.movesNum, FALSE);
    }

    r->stack = Stack_New (StackFrame, NULL);
    // FIXME: Artyom Goncharov TAB_table does not handle primitive types
    // for now we gonna use additional conversion
    r->aliases = TAB_Empty();
    r->coloring = coloring;
    r->colors = NULL;
    r->accessList = accessList;
    // FIXME Artyom Goncharov there is no need to create reg map every time
    r->results = Temp_LayerMap (Temp_Empty(), F_RegistersToMap (Temp_Empty(), regs_all));

    return r;
}

static char * Workspace_ToString (Workspace w)
{
    // TODO: Artyom Goncharov this number is totally arbitrary, needs to be revised
    char * r = checked_malloc (1024 * 610);
    int length = 0;

    length += sprintf (&r[length], "temps:");
    for (Temp_tempList l = w->temps; l; l = l->tail)
    {
        length += sprintf (&r[length], " %d", Temp_GetTempIndex (l->head));
    }
    /* length += sprintf (&r[length], "\ninterference graph:\n%s", BitMatrix_ToString (w->interference)); */
    length += sprintf (&r[length], "\nprecolored: %s", BitArray_ToString (w->precolored));
    length += sprintf (&r[length], "\ninitial:    %s", BitArray_ToString (w->initial));
    length += sprintf (&r[length], "\nsimplify:   %s", BitArray_ToString (w->simplify));
    length += sprintf (&r[length], "\nfreezify:   %s", BitArray_ToString (w->freezify));
    length += sprintf (&r[length], "\nspillify:   %s", BitArray_ToString (w->spillify));
    length += sprintf (&r[length], "\nspilled:    %s", BitArray_ToString (w->spilled));
    length += sprintf (&r[length], "\ncoalesced:  %s", BitArray_ToString (w->coalesced));
    r[length] = '\0';

    return r;
}

static void Workspace_Print (FILE * out, Workspace w)
{
    fprintf (out, "temps:");
    for (Temp_tempList l = w->temps; l; l = l->tail)
    {
        fprintf (out, " %d", Temp_GetTempIndex (l->head));
    }
    fprintf (out, "\n");
    fprintf (out, "interference graph:\n");
    BitMatrix_Print (out, w->interference);
    fprintf (out, "initial:   ");
    BitArray_Print (out, w->initial);
    fprintf (out, "simplify:  ");
    BitArray_Print (out, w->simplify);
    fprintf (out, "freezify:  ");
    BitArray_Print (out, w->freezify);
    fprintf (out, "spillify:  ");
    BitArray_Print (out, w->spillify);
    fprintf (out, "spilled:   ");
    BitArray_Print (out, w->spilled);
    fprintf (out, "coalesced: ");
    BitArray_Print (out, w->coalesced);
    fprintf (out, "\n");
}

/* static void Workspace_Reset (Workspace w) */
/* { */
/*  */
/* } */

static void Workspace_Delete (Workspace w)
{
    // TODO: Artyom Goncharov proper destroy here please
    (void) w;
}

static BitArray NodeMoves (Workspace w, Temp_temp t)
{
    if (!w->movesTable)
    {
        return NULL;
    }
    BitArray a = (BitArray)TAB_Look (w->movesTable, t);
    if (!a)
    {
        return NULL;
    }

    BitArray r = BitArray_New (w->movesNum, FALSE);
    BitArray_Union (w->activeMoves, w->worklistMoves, r);
    BitArray_Intersection (a, r, r);

    return r;
}

static int GetAlias (Workspace w, int n)
{
    if (BitArray_IsSet (w->coalesced, n))
    {
        // FIXME: Artyom Goncharov not addition conversion
        Temp_temp ntmp = GetTemp (n, w->temps);
        int index = GetIndex ((Temp_temp)TAB_Look (w->aliases, ntmp), w->temps);
        return GetAlias (w, index);
    }
    return n;
}

static bool IsMoveRelated (Workspace w, Temp_temp t)
{
    BitArray moves = NodeMoves (w, t);
    return moves && BitArray_HasSet (moves);
}

static void MakeWorkLists (Workspace w)
{
    int index = w->initial->length;
    while (index--)
    {
        if (BitArray_IsSet (w->initial, index))
        {
            BitArray_UnSet (w->initial, index);

            /**
             * We need to check whether the processed node has already
             * a color, if so we push it the result map immediately
             */
            Temp_temp t = (Temp_temp)GetTemp (index, w->temps);
            Temp_temp color = (Temp_temp)TAB_Look (w->coloring, t);
            if (color)
            {
                TAB_Enter (w->coloring, t, color);
                Temp_Enter (w->results, t, (char *)Temp_Look (w->results, color));
            }
            else
            {
                int degree = BitMatrix_GetRowDegree (w->interference, index);
                if (degree >= w->K)
                {
                    BitArray_Set (w->spillify, index);
                }
                else
                {
                    if (IsMoveRelated (w, t))
                    {
                        BitArray_Set (w->freezify, index);
                    }
                    else
                    {
                        BitArray_Set (w->simplify, index);
                    }
                }
            }

        }
    }
}

// TODO can be simplified with getting a column and diff with coalesced
static BitArray Adjacent (Workspace w, int temp)
{
    BitArray r = BitMatrix_GetRowCopy (w->interference, temp, NULL);
    BitArray s = BitArray_New (w->interference->width, FALSE);
    // FIXME: Artyom Goncharov not pretty
    int len = Stack_Size (w->stack);
    StackFrame * f = (StackFrame *)w->stack->items;
    while (len--)
    {
        BitArray_Set (s, f[len].index);
    }
    BitArray_Union (s, w->coalesced, s);
    BitArray_RelDifference (r, s, r);
    BitArray_Delete (s);
    return r;
}

static void EnableMoves (Workspace w, BitArray nodes)
{
    BITARRAY_FOREACH_SET (n, nodes)
    {
        BitArray moves = NodeMoves (w, GetTemp (n, w->temps));
        if (!moves)
        {
            continue;
        }
        BITARRAY_FOREACH_SET (m, moves)
        {
            if (BitArray_IsSet (w->activeMoves, m))
            {
                BitArray_UnSet (w->activeMoves, m);
                BitArray_Set (w->worklistMoves, m);
            }
        }
    }
}

static void UpdateMoveLists (Workspace w, int m)
{
    DBG ("UpdateMoveLists: %d\n", m);
    // TODO Artyom Goncharov is this the right way to do it
    if (BitArray_IsSet (w->precolored, m))
    {
        return;
    }
    int degree = BitMatrix_GetRowDegree (w->interference, m);
    // TODO: Artyom Goncharov is the compare correct here?
    if (degree == w->K)
    {
        BitArray nodes = BitArray_New (w->simplify->length, FALSE);
        BitArray_Set (nodes, m);
        BitArray_Union (nodes, Adjacent (w, m), nodes);
        EnableMoves (w, nodes);
        BitArray_UnSet (w->spillify, m);

        if (IsMoveRelated (w, GetTemp (m, w->temps)))
        {
            BitArray_Set (w->freezify, m);
        }
        else
        {
            BitArray_Set (w->simplify, m);
        }
    }
}

static void Simplify (Workspace w)
{
    DBG ("%s", "Simplify:");
    BITARRAY_FOREACH_SET (index, w->simplify)
    {
        DBG (" %d\n", index);
        BitArray_UnSet (w->simplify, index);
        StackFrame sf;
        sf.index = index;
        sf.edges = BitMatrix_GetCollumnCopy (w->interference, index, NULL);
        BitMatrix_UnSetColumn (w->interference, index);
        Stack_Push (w->stack, sf);
        BitArray adj = Adjacent (w, index);
        BITARRAY_FOREACH_SET (a, adj)
        {
            UpdateMoveLists (w, a);
        }

        return;
    }
}

static void EnableForSimplification (Workspace w, int temp)
{
    if (!BitArray_IsSet (w->precolored, temp)
            && !IsMoveRelated (w, GetTemp (temp, w->temps))
            && BitMatrix_GetRowDegree (w->interference, temp) < w->K)
    {
        DBG ("EnableForSimplification: %d\n", temp);
        BitArray_UnSet (w->freezify, temp);
        BitArray_Set (w->simplify, temp);
    }
}

static bool OK (Workspace w, int t, int r)
{
    return BitMatrix_GetRowDegree (w->interference, t) < w->K
           || BitArray_IsSet (w->precolored, t)
           || BitMatrix_IsSet (w->interference, t, r);
}

static bool ForAllOK (Workspace w, BitArray t, int u)
{
    for (int i = 0; i < t->length; ++i)
    {
        if (BitArray_IsSet (t, i))
        {
            if (!OK (w, i, u))
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

static bool Conservative (Workspace w, int u, int v)
{
    BitArray ua = Adjacent (w, u);
    BitArray va = Adjacent (w, v);

    BitArray_Union (ua, va, ua);
    BitArray_Delete (va);

    int k = 0;
    for (int i = 0; i < ua->length; ++i)
    {
        if (BitArray_IsSet (ua, i) && BitMatrix_GetRowDegree (w->interference, i) >= w->K)
        {
            k++;
        }
    }

    return k < w->K;
}

static void Combine (Workspace w, int u, int v)
{
    if (BitArray_IsSet (w->freezify, v))
    {
        BitArray_UnSet (w->freezify, v);
    }
    else
    {
        BitArray_UnSet (w->spillify, v);
    }

    BitArray_Set (w->coalesced, v);

    Temp_temp ut = GetTemp (u, w->temps);
    Temp_temp vt = GetTemp (v, w->temps);
    TAB_Enter (w->aliases, vt, ut);

    BitArray umb = (BitArray)TAB_Look (w->movesTable, ut);
    BitArray vmb = (BitArray)TAB_Look (w->movesTable, vt);
    BitArray_Union (umb, vmb, umb);

    UpdateMoveLists (w, v);

    BitArray va = Adjacent (w, v);
    DBG ("%d adj: %s\n", v, BitArray_ToString (va));
    BITARRAY_FOREACH_SET (t, va)
    {
        BitMatrix_UnSet (w->interference, t, v);
        BitMatrix_UnSet (w->interference, v, t);

        if (t != u)
        {
            BitMatrix_Set (w->interference, t, u);
            BitMatrix_Set (w->interference, u, t);
        }

        UpdateMoveLists (w, t);
    }

    if (BitMatrix_GetRowDegree (w->interference, u) >= w->K && BitArray_IsSet (w->freezify, u))
    {
        BitArray_UnSet (w->freezify, u);
        BitArray_Set (w->spillify, u);
    }
}

static void Coalesce (Workspace w)
{
    DBG ("%s", "Coalesce");
    BITARRAY_FOREACH_SET (first, w->worklistMoves)
    {
        BitArray_UnSet (w->worklistMoves, first);

        ASM_line move = GetMove (first, w->moveList);
        // SHIT: u know what to do
        int di = GetIndex (move->u.MOVE.dst->head, w->temps);
        int si = GetIndex (move->u.MOVE.src->head, w->temps);
        int x = GetAlias (w, di);
        int y = GetAlias (w, si);
        int u, v;

        if (BitArray_IsSet (w->precolored, y))
        {
            u = y;
            v = x;
        }
        else
        {
            u = x;
            v = y;
        }

        DBG (" move: %d, u: %d, v: %d\n", first, u, v);


        if (u == v)
        {
            DBG ("rule: equal\n");
            BitArray_Set (w->coalescedMoves, first);
            EnableForSimplification (w, u);
        }
        else if (BitArray_IsSet (w->precolored, v)
                 || BitMatrix_IsSet (w->interference, u, v))
        {
            DBG ("rule: constrained\n");
            BitArray_Set (w->constrainedMoves, first);
            EnableForSimplification (w, u);
            EnableForSimplification (w, v);
        }
        else if ((BitArray_IsSet (w->precolored, u) && ForAllOK (w, Adjacent (w, v), u))
                 || (!BitArray_IsSet (w->precolored, u) && Conservative (w, u, v)))
        {
            DBG ("rule: magic\n");
            BitArray_Set (w->coalescedMoves, first);
            Combine (w, u, v);
            EnableForSimplification (w, u);
        }
        else
        {
            DBG ("rule: not active yet\n");
            BitArray_Set (w->activeMoves, first);
        }

        // proccess a single node per function call
        return;
    }
}

static void FreezeMoves (Workspace w, int u)
{
    BitArray moves = NodeMoves (w, GetTemp (u, w->temps));
    if (!moves)
    {
        return;
    }

    BITARRAY_FOREACH_SET (i, moves)
    {
        ASM_line line = GetMove (i, w->moveList);
        Temp_temp x = line->u.MOVE.dst->head;
        Temp_temp y = line->u.MOVE.src->head;
        int v;
        if (GetAlias (w, GetIndex (y, w->temps)) == GetAlias (w, u))
        {
            v = GetAlias (w, GetIndex (x, w->temps));
        }
        else
        {
            v = GetAlias (w, GetIndex (y, w->temps));
        }

        BitArray_UnSet (w->activeMoves, i);
        BitArray_Set (w->frozenMoves, i);

        Temp_temp vt = GetTemp (v, w->temps);

        if (!BitArray_HasSet ((BitArray)TAB_Look (w->movesTable, vt))
                && BitMatrix_GetRowDegree (w->interference, v) < w->K)
        {
            BitArray_UnSet (w->freezify, v);
            BitArray_Set (w->simplify, v);
        }
    }
}

static void Freeze (Workspace w)
{
    DBG ("%s", "Freeze:");
    if (BitArray_HasSet (w->freezify))
    {
        int first = BitArray_FirstSet (w->freezify);
        DBG (" %d\n", first);
        BitArray_UnSet (w->freezify, first);
        BitArray_Set (w->simplify, first);
        FreezeMoves (w, first);
    }
}

static void Spillify (Workspace w)
{
    //TODO should be chosen more wisely
    DBG ("%s", "Spillify:");
    BITARRAY_FOREACH_SET (i, w->spillify)
    {
        DBG (" %d\n", i);
        BitArray_UnSet (w->spillify, i);
        BitArray_Set (w->simplify, i);
        FreezeMoves (w, i);

        // we need to process only a single node per call
        return;
    }
}

static void Colorify (Workspace w)
{
    DBG ("%s\n", "Colorify");
    Stack s = w->stack;
    BitArray interference = BitArray_New (w->interference->width, FALSE);
    BitArray colored = BitArray_New (w->simplify->length, FALSE);
    while (!Stack_Empty (s))
    {
        StackFrame * sf = Stack_Top (s);
        Temp_temp temp = GetTemp (sf->index, w->temps);

        /*
         * Putting back the column so that we could evaluate nodes' degree
         * correctly
         */
        BitMatrix_SetColumnFromArray (w->interference, sf->edges, sf->index);
        /**
         * Set of colors that is possible to use. It is updated (decremented) with
         * each colorized adjacent node of the current node.
         */
        BitArray freeColors = BitArray_New (w->K, TRUE);
        /**
         * Adjacency list of the current node
         */
        // this line reads all available interference in the i-graph for this node
        /* interference = sf.edges; */
        // this line reads currently available interference for this node
        BitMatrix_GetCollumnCopy (w->interference, sf->index, interference);
        BITARRAY_FOREACH_SET (i, interference)
        {
            int a = GetAlias (w, i);
            if (BitArray_IsSet (colored, a) || BitArray_IsSet (w->precolored, a))
            {
                Temp_temp t = GetTemp (a, w->temps);
                Temp_temp color = (Temp_temp)TAB_Look (w->coloring, t);
                if (color)
                {
                    int index = GetIndex (color, w->regs);
                    BitArray_UnSet (freeColors, index);
                }
            }
            /* DBG ("interference: %s; %d alias to %d\n", BitArray_ToString (interference), a, i); */
            /* #<{(|* */
            /*  * Check whether current node has an edge with the processed node and */
            /*  * is colored. If so, we will get the index of the color and remove it */
            /*  * from the list of free colors. */
            /*  |)}># */
            /* if (BitArray_IsSet (interference, a)) */
            /* { */
            /* } */
        }

        /**
         * Looking for a free color, if there is one we map it, otherwise we need to
         * spill the temp
         */
        Temp_temp color = NULL;
        BITARRAY_FOREACH_SET (i, freeColors)
        {
            color = GetTemp (i, w->regs);
            DBG ("temp %d colored as %s\n", sf->index, Temp_Look (w->results, color));
            BitArray_Set (colored, sf->index);
            LIST_PUSH_UNIQUE (w->colors, color);
            TAB_Enter (w->coloring, temp, color);
            Temp_Enter (w->results, temp, (char *)Temp_Look (w->results, color));
            break;
        }

        if (!color)
        {
            DBG ("temp %d spilled\n", sf->index);
            BitArray_Set (w->spilled, sf->index);
        }

        Stack_Pop (s);
    }

    BITARRAY_FOREACH_SET (c, w->coalesced)
    {
        Temp_temp ct = GetTemp (c, w->temps);
        int a = GetAlias (w, c);
        Temp_temp at = GetTemp (a, w->temps);
        Temp_temp color = (Temp_temp)TAB_Look (w->coloring, at);
        if (!color && BitArray_IsSet (w->spilled, a))
        {
            // HMM is it the right place to solve spilled coalesce?
            // looks like no action here
        }
        else
        {
            TAB_Enter (w->coloring, ct, color);
            Temp_Enter (w->results, ct, (char *)Temp_Look (w->results, color));
            DBG ("coalesced temp %d colored as %s\n", c, Temp_Look (w->results, color));
        }
    }
}

static ASM_lineList Rewrite (Workspace w)
{
    DBG ("Rewrite");
    DBG (".before:\n%s\n", ASM_LineListToString (w->asmll, Temp_Name()));

    /*
     * We need to allocate space on stack to store the spilled temps.
     * TODO stack space coloring, like for registers
     */
    size_t spilledNum = BitArray_GetDegree (w->spilled);
    size_t accessNum = LIST_SIZE (w->accessList);
    while (accessNum < spilledNum)
    {
        LIST_PUSH (w->accessList, F_AllocFrame (w->frame, "SPILL"));
        accessNum++;
    }

    DBG ("additional stack space: %lu\n", accessNum);

    ASM_lineList result = w->asmll;
    F_access access = NULL;
    BITARRAY_FOREACH_SET (index, w->spilled)
    {
        DBG ("replace temp %d\n", index);

        BitArray_UnSet (w->spilled, index);

        Temp_temp temp = GetTemp (index, w->temps);

        accessNum--;
        access = LIST_AT (w->accessList, accessNum);

        ASM_lineList pre = NULL;
        ASM_lineList cur = w->asmll;
        for (; cur; pre = cur, cur = cur->tail)
        {
            ASM_line line = cur->head;
            if (line->kind == I_LABEL || line->kind == I_META)
            {
                continue;
            }

            Temp_tempList dl, ul;
            // SHIT: get rid of ifs for fuck sake
            if (line->kind == I_OPER)
            {
                dl = line->u.OPER.dst;
                ul = line->u.OPER.src;
            }
            else
            {
                dl = line->u.MOVE.dst;
                ul = line->u.MOVE.src;
            }

            // TODO: make it available through Flow_graph ds for quick lookup
            BitArray use = MapTempList (w->flow->tempsNum, ul, w->temps);
            BitArray dst = MapTempList (w->flow->tempsNum, dl, w->temps);
            /////////

            Temp_temp v = NULL;

            if (BitArray_IsSet (use, index))
            {
                /*
                 * Each use occurrence of the 'temp' must be replaced with the new 'v'
                 * temp and prefixed with fetch instruction
                 */
                v = Temp_NewTemp();
                for (; ul; ul = ul->tail)
                {
                    if (ul->head == temp)
                    {
                        ul->head = v;
                    }
                }

                T_stmList stms = NULL;
                LIST_PUSH (stms, T_Move (T_Temp (v), F_GetVar (access, T_Temp (F_FP()), TRUE)));

                ASM_lineList fll = F_CodeGen (w->frame, stms);

                LIST_JOIN (fll, cur);
                if (pre)
                {
                    pre->tail = fll;
                }
                else
                {
                    result = pre = fll;
                }
            }

            if (BitArray_IsSet (dst, index))
            {
                /*
                 * We want to use the same 'v' temp in case 'temp' is used as source
                 * and destination at the same time
                 */
                if (!v)
                {
                    v = Temp_NewTemp();
                }
                /*
                 * Here we are replacing the occurrences of the 'v' temp with the newly
                 * created temporary. Current instruction will postfixed with a store
                 * instruction
                 */
                for (; dl; dl = dl->tail)
                {
                    if (dl->head == temp)
                    {
                        dl->head = v;
                    }
                }

                T_stmList stms = NULL;
                LIST_PUSH (stms, T_Move (F_GetVar (access, T_Temp (F_FP()), TRUE), T_Temp (v)));
                ASM_lineList sll = F_CodeGen (w->frame, stms);

                LIST_JOIN (sll, cur->tail);
                cur->tail = sll;
            }
        }
    }

    DBG (".after:\n%s\n", ASM_LineListToString (result, Temp_Name()));

    return result;
}

RA_Result RA_RegAlloc (F_frame f, ASM_lineList ll, F_registers regs_all, F_registers regs_colors)
{
    DBG ("\n%s", "RA_RegAlloc");
    Workspace w = NULL;
    /**
     * The loop runs until we successfully colored the whole code fragment
     */
    while (TRUE)
    {
        DBG ("%s", ".START\n");
        w = Workspace_New (f, ll, regs_all, regs_colors, w ? w->accessList : NULL);
        MakeWorkLists (w);

        DBG ("Fragment:\n%s\n", ASM_LineListToString (ll, Temp_Name()));
        DBG ("Initial Workspace:\n%s\n\n", Workspace_ToString (w));
        /**
         * The loop lasts until we colored every possible node including
         * some spilled nodes. The rest of the nodes are meant to be spilled
         * and the whole program will be rewritten.
         */
        while (BitArray_HasSet (w->simplify)
                || (w->worklistMoves && BitArray_HasSet (w->worklistMoves))
                || BitArray_HasSet (w->freezify)
                || BitArray_HasSet (w->spillify))
        {
            if (BitArray_HasSet (w->simplify))
            {
                Simplify (w);
                DBG ("After Simplify Workspace:\n%s\n\n", Workspace_ToString (w));
            }
            else if (w->worklistMoves && BitArray_HasSet (w->worklistMoves))
            {
                Coalesce (w);
                DBG ("After Coalesce Workspace:\n%s\n\n", Workspace_ToString (w));
            }
            else if (BitArray_HasSet (w->freezify))
            {
                Freeze (w);
                DBG ("After Freeze Workspace:\n%s\n\n", Workspace_ToString (w));
            }
            else if (BitArray_HasSet (w->spillify))
            {
                Spillify (w);
                DBG ("After Spillify Workspace:\n%s\n\n", Workspace_ToString (w));
            }
        }
        Colorify (w);
        DBG ("After Colorify Workspace:\n%s\n\n", Workspace_ToString (w));

        if (BitArray_HasSet (w->spilled))
        {
            ll = Rewrite (w);
            Workspace_Delete (w);
        }
        else
        {
            break;
        }
    }

    /* printf("colors:"); */
    /* LIST_FOREACH(item, w->colors) */
    /* { */
    /*     printf(" %s", Temp_Look(w->results, item)); */
    /* } */
    /* printf("\n"); */

    /* Temp_DumpMap (stdout, w->results); */
    RA_Result rar = checked_malloc (sizeof * rar);
    rar->coloring = w->results;
    rar->colors = w->colors;
    rar->il = ll;
    return rar;
}
