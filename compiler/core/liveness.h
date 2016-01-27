#ifndef LIVENESS_H_FSGJVL1R
#define LIVENESS_H_FSGJVL1R

#include "core/flowgraph.h"
#include "core/graph.h"
#include "core/temp.h"

typedef struct Live_moveList_ * Live_moveList;
struct Live_moveList_
{
    Live_moveList tail;
    ASM_line line;
    G_node src, dst;
};

Live_moveList Live_MoveList (ASM_line line, G_node src, G_node dst, Live_moveList tail);

struct Live_graph
{
    /**
     * Interference graph
     */
    G_graph graph;
    /**
     * Move instructions to be coalesced
     */
    Live_moveList moves;
    int movesNum;
};

Temp_temp Live_Gtemp (G_node n);

struct Live_graph Live_Liveness (Flow_graph flow);

#endif /* end of include guard: LIVENESS_H_FSGJVL1R */

