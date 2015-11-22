#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "asm.h"
#include "temp.h"
#include "flowgraph.h"
#include "liveness.h"
#include "regalloc.h"

static void liveness_ok (void ** state)
{
    Temp_temp a = Temp_NewTemp();
    Temp_temp b = Temp_NewTemp();
    Temp_temp c = Temp_NewTemp();

    Temp_label l1 = Temp_NewLabel ();

    /*
     * Program:
     *
     * a = 0
     * l1:
     * b = a + 1
     * c = c + b
     * a = b * 2
     * if a < 0
     *  goto l1
     * return c
     */

    ASM_lineList list =
        ASM_LineList (ASM_Oper ("li  `d0, 0", Temp_TempList(a, NULL), NULL, NULL),
        ASM_LineList (ASM_Label ("l1: ", l1),
        ASM_LineList (ASM_Oper ("add `d0, `s0, 1", Temp_TempList (b, NULL), Temp_TempList (a, NULL), NULL),
        ASM_LineList (ASM_Oper ("add `d0, `s0, `s1", Temp_TempList (c, NULL), Temp_TempList (c, Temp_TempList (b, NULL)), NULL),
        ASM_LineList (ASM_Oper ("mul `d0, `s0, 2", Temp_TempList (a, NULL), Temp_TempList (b, NULL), NULL),
        ASM_LineList (ASM_Oper ("blz `s0, l1", NULL, Temp_TempList (a, NULL), ASM_Targets (Temp_LabelList (l1, NULL))),
        ASM_LineList (ASM_Move ("mov $v0, `s0", NULL, Temp_TempList (c, NULL)), NULL)))))));

    Flow_graph fg = FG_AsmFlowGraph (list);
    assert_non_null (fg);

    /*
     * Liveness analysis:  Interference:       +-+
     *                                         |c|
     *     out  in          a(0) c(2)          +-+
     * 6 |       c          b(1) c(2)         /   \
     * 5 | ac   ac          c(2) c(2)      +-+     +-+
     * 4 | ac   bc                         |a|     |b|
     * 3 | bc   bc                         +-+     +-+
     * 2 | bc   ac
     * 1 | ac    c
     */
    struct Live_graph lg = Live_Liveness (fg);

    G_nodeList nodes = G_Nodes (lg.graph);
    G_node na = nodes->head;
    G_node nb = nodes->tail->head;
    G_node nc = nodes->tail->tail->head;

    printf ("flow graph\n%s\n", G_ToString (G_Nodes (fg->graph)));
    printf ("intr graph\n%s\n", G_ToString (G_Nodes (lg.graph)));

    G_Show(stdout, G_Nodes(lg.graph), NULL);

    assert_true (G_NodeInfo (na) == a);
    assert_true (G_NodeInfo (nb) == b);
    assert_true (G_NodeInfo (nc) == c);

    // a ~ b -> false
    assert_false (G_Conn (na, nb));
    assert_false (G_Conn (nb, na));

    // c ~ a -> true
    assert_true (G_Conn (nc, na));
    assert_true (G_Conn (na, nc));

    // c ~ a -> true
    assert_true (G_Conn (nb, nc));
    assert_true (G_Conn (nc, nb));

    // self - > false
    assert_false (G_Conn (na, na));
    assert_false (G_Conn (nb, nb));
    assert_false (G_Conn (nc, nc));

    (void) state;
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (liveness_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
