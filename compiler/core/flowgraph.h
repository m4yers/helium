#ifndef FLOWGRAPH_H_LRFKC0ON
#define FLOWGRAPH_H_LRFKC0ON

#include "core/graph.h"
#include "core/temp.h"
#include "core/asm.h"

typedef struct
{
    G_graph graph;
    Temp_tempList temps;
    int tempsNum;
} * Flow_graph;

Temp_tempList FG_Def (G_node n);
Temp_tempList FG_Use (G_node n);
bool FG_IsMove (G_node n);

Flow_graph FG_AsmFlowGraph (ASM_lineList list);

#endif /* end of include guard: FLOWGRAPH_H_LRFKC0ON */
