#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))
#define ASSERT_MAPPING(result, temp, reg) assert_true (Temp_Look (result.coloring, temp) == Temp_Look (map, reg));
#define ASSERT_NO_MAPPING(result, temp) assert_true (Temp_Look (result.coloring, temp) == NULL);

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "asm.h"
#include "temp.h"
#include "regalloc.h"
#include "frame.h"
#include "mipsmachine.h"
#include "util.h"

#define L Temp_TempList

F_registers test_regs_colors;
Temp_map map;

static void InitMachine()
{
    F_Init();

    // i do not want to use all reg set

    test_regs_colors = F_Registers (NULL, NULL);
    F_RegistersAdd (test_regs_colors, v0, "$v0");
    F_RegistersAdd (test_regs_colors, a0, "$r0");
    /* F_RegistersAdd (test_regs_colors, a1, "$r1"); */
    /* F_RegistersAdd (test_regs_colors, a2, "$r2"); */

    map = F_RegistersToMap (Temp_Empty(), regs_all);
}

static void regalloc_simple_ok (void ** state)
{
    Temp_temp a = Temp_NewTemp();
    Temp_temp b = Temp_NewTemp();
    Temp_temp c = Temp_NewTemp();

    Temp_label l1 = Temp_NewLabel();

    ASM_lineList list =
        ASM_LineList (ASM_Oper ("li  `d0, 0", L (a, NULL), NULL, NULL),
                      ASM_LineList (ASM_Label ("l1: ", l1),
                                    ASM_LineList (ASM_Oper ("add `d0, `s0, 1", L (b, NULL), L (a, NULL), NULL),
                                            ASM_LineList (ASM_Oper ("add `d0, `s0, `s1", L (c, NULL), L (c, L (b, NULL)), NULL),
                                                    ASM_LineList (ASM_Oper ("mul `d0, `s0, 2", L (a, NULL), L (b, NULL), NULL),
                                                            ASM_LineList (ASM_Oper ("blz `s0, l1", NULL, L (a, NULL), ASM_Targets (Temp_LabelList (l1, NULL))),
                                                                    ASM_LineList (ASM_Move ("mov $$, `s0", NULL, L (c, NULL)), NULL)))))));

    struct RA_result r = RA_RegAlloc (NULL, list, regs_all, test_regs_colors);
    ASM_PrintLineList (stdout, list, Temp_Name());
    ASM_PrintLineList (stdout, list, r.coloring);

    ASSERT_MAPPING (r, a, a0);
    ASSERT_MAPPING (r, b, a0);
    ASSERT_MAPPING (r, c, v0);

    (void) state;
}

static void regalloc_precolored_ok (void ** state)
{
    Temp_temp a = a1;
    Temp_temp b = Temp_NewTemp();
    Temp_temp c = Temp_NewTemp();

    Temp_label l1 = Temp_NewLabel();

    ASM_lineList list =
        ASM_LineList (ASM_Oper ("li  `d0, 0", L (a, NULL), NULL, NULL),
                      ASM_LineList (ASM_Label ("l1: ", l1),
                                    ASM_LineList (ASM_Oper ("add `d0, `s0, 1", L (b, NULL), L (a, NULL), NULL),
                                            ASM_LineList (ASM_Oper ("add `d0, `s0, `s1", L (c, NULL), L (c, L (b, NULL)), NULL),
                                                    ASM_LineList (ASM_Oper ("mul `d0, `s0, 2", L (a, NULL), L (b, NULL), NULL),
                                                            ASM_LineList (ASM_Oper ("blz `s0, l1", NULL, L (a, NULL), ASM_Targets (Temp_LabelList (l1, NULL))),
                                                                    ASM_LineList (ASM_Move ("mov $$, `s0", NULL, L (c, NULL)), NULL)))))));

    struct RA_result r = RA_RegAlloc (NULL, list, regs_all, test_regs_colors);
    ASSERT_MAPPING (r, b, a0);
    ASSERT_MAPPING (r, c, v0);

    (void) state;
}

/**
 * Optimistic in regard that there are nodes with K+ degree and
 * it is still possible to color the graph with K colors without
 * the actual spilling
 */
static void regalloc_optimistic_spilling_ok (void ** state)
{
    Temp_temp a = Temp_NewTemp();
    Temp_temp b = Temp_NewTemp();
    Temp_temp c = Temp_NewTemp();
    Temp_temp d = Temp_NewTemp();
    Temp_temp e = Temp_NewTemp();

    Temp_label l1 = Temp_NewLabel();

    ASM_lineList list =
        ASM_LineList (ASM_Oper ("li  `d0, 0", L (a, NULL), NULL, NULL),
                      ASM_LineList (ASM_Label ("l1: ", l1),
                                    ASM_LineList (ASM_Oper ("add `d0, `s0, 1", L (b, NULL), L (a, NULL), NULL),
                                            ASM_LineList (ASM_Oper ("add `d0, `s0, 1", L (d, NULL), L (b, NULL), NULL),
                                                    ASM_LineList (ASM_Oper ("add `d0, `s0, 1", L (e, NULL), L (d, NULL), NULL),
                                                            ASM_LineList (ASM_Oper ("add `d0, `s0, `s1", L (c, NULL), L (c, L (b, NULL)), NULL),
                                                                    ASM_LineList (ASM_Oper ("mul `d0, `s0, 2", L (a, NULL), L (b, NULL), NULL),
                                                                            ASM_LineList (ASM_Oper ("blz `s0, l1", NULL, L (a, NULL), ASM_Targets (Temp_LabelList (l1, NULL))),
                                                                                    ASM_LineList (ASM_Move ("mov $$, `s0", NULL, L (c, NULL)), NULL)))))))));

    struct RA_result r = RA_RegAlloc (NULL, list, regs_all, test_regs_colors);
    ASSERT_MAPPING (r, a, a0);
    ASSERT_MAPPING (r, b, a1);
    ASSERT_MAPPING (r, c, v0);
    ASSERT_MAPPING (r, d, a0);
    ASSERT_MAPPING (r, e, a0);

    (void) state;
}

static void regalloc_real_spilling_ok (void ** state)
{
    Temp_temp a = Temp_NewTemp();
    Temp_temp b = Temp_NewTemp();
    Temp_temp c = Temp_NewTemp();

    F_frame frame = F_NewFrame (Temp_NamedLabel ("__blah"), NULL);

    Temp_label l1 = Temp_NewLabel();

    /*
     * a = 0
     *
     * do
     * {
     *    b = a + 1
     *    c = c + b
     *    a = b * 2
     *    c = b * 2
     * }
     * while (a < 0)
     *
     * return c
     */
    ASM_lineList list = NULL;
    LIST_PUSH (list, ASM_Oper ("li  `d0, 0", L (a, NULL), NULL, NULL))
    LIST_PUSH (list, ASM_Label ("l1: ", l1))
    LIST_PUSH (list, ASM_Oper ("add `d0, `s0, 1", L (b, NULL), L (a, NULL), NULL))
    LIST_PUSH (list, ASM_Oper ("add `d0, `s0, `s1", L (c, NULL), L (c, L (b, NULL)), NULL))
    LIST_PUSH (list, ASM_Oper ("mul `d0, `s0, 2", L (a, NULL), L (b, NULL), NULL))
    LIST_PUSH (list, ASM_Oper ("mul `d0, `s0, 2", L (c, NULL), L (b, NULL), NULL))
    LIST_PUSH (list, ASM_Oper ("blz `s0, l1", NULL, L (a, NULL), ASM_Targets (Temp_LabelList (l1, NULL))));
    LIST_PUSH (list, ASM_Move ("mov `d0 `s0", L (v0, NULL), L (c, NULL)));

    struct RA_result r = RA_RegAlloc (frame, list, regs_all, test_regs_colors);
    ASM_PrintLineList (stdout, list, Temp_Name());
    ASM_PrintLineList (stdout, list, r.coloring);

    /* ASSERT_MAPPING (r, a, a0); */
    /* ASSERT_MAPPING (r, b, a0); */
    ASSERT_MAPPING (r, c, a0);

    (void) state;
}

int main (void)
{
    InitMachine();
    const struct CMUnitTest tests[] =
    {
        /* cmocka_unit_test (regalloc_simple_ok), */
        /* cmocka_unit_test (regalloc_precolored_ok), */
        /* cmocka_unit_test (regalloc_optimistic_spilling_ok), */
        cmocka_unit_test (regalloc_real_spilling_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
