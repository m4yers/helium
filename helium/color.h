#ifndef COLOR_H_SRMTOX0V
#define COLOR_H_SRMTOX0V

#include "temp.h"
#include "graph.h"
#include "liveness.h"

struct COL_result
{
    Temp_map coloring;
    Temp_tempList spills;
};
struct COL_result COL_color (struct Live_graph lg, Temp_map initial, Temp_tempList regs);

#endif /* end of include guard: COLOR_H_SRMTOX0V */


