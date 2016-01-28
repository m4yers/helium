#pragma once

#include <stdint.h>

#include "util/list.h"
#include "util/bool.h"
#include "util/util.h"

#include "core/machine.h"
#include "core/temp.h"
#include "core/ir.h"

// FIXME const parameters

/*************
*  Closure  *
*************/

/*
 * Closure in context of MIPS IR(and any ASM in general means stuff that needs to happen to
 * successfully use an opperand or invoke instruction. It can be either pre/post ASM statement or
 * .data segment definition or virtually anything. This also can be thought of as an execution
 * context of some sort.
 */

typedef enum
{
    /*
     * These two kinds require pre/post Helium IR evaluation, it is used for vars interpolation etc.
     */
    IR_mipsClosureHeStmPreKind,
    IR_mipsClosureHeStmPostKind,
} IR_mipsClosure_k;

typedef struct IR_mipsClosureItem_t
{
    IR_mipsClosure_k kind;

    union
    {
        const struct T_stm_t * heStmPre;
        const struct T_stm_t * heStmPost;
    } u;
} * IR_mipsClosureItem;

static inline IR_mipsClosureItem IR_MipsClosureHeStm (const struct T_stm_t * stm, bool is_pre)
{
    U_Create (IR_mipsClosureItem, r)
    {
        .kind = is_pre ? IR_mipsClosureHeStmPreKind : IR_mipsClosureHeStmPostKind,
         .u.heStmPre = stm
    };
    return r;
}

LIST_DEFINE (IR_mipsClosure, IR_mipsClosureItem)

/**************
*  Operands  *
**************/

typedef enum
{
    IR_mipsImmInt32Kind,
    IR_mipsImmUInt32Kind,
} IR_mipsImmKind;

typedef struct IR_mipsOpdImm_t
{
    IR_mipsImmKind kind;
    union
    {
        const uint32_t uval;
        const int32_t ival;
    } u;
} * IR_mipsOpdImm;

typedef struct IR_mipsOpdLab_t
{
    Temp_label label;
} * IR_mipsOpdLab;

typedef enum
{
    IR_mipsOpdRepDstKind,
    IR_mipsOpdRepSrcKind,
    IR_mipsOpdRepJmpKind
} IR_mipsOpdRep_k;

typedef struct IR_mipsOpdRep_t
{
    IR_mipsOpdRep_k kind;

    union
    {
        Temp_temp tmp;
        Temp_label lab;
    } u;
} * IR_mipsOpdRep;

// forward declaration
typedef struct IR_mipsOpd_t * IR_mipsOpd;

typedef struct IR_mipsOpdMem_t
{
    intmax_t offset;
    IR_mipsOpd base;
} * IR_mipsOpdMem;

typedef enum
{
    IR_mipsOpdImmKind,
    IR_mipsOpdMemKind,
    IR_mipsOpdLabKind,
    IR_mipsOpdRepKind
} IR_mipsOpd_k;

struct IR_mipsOpd_t
{
    IR_mipsOpd_k kind;

    union
    {
        const struct IR_mipsOpdImm_t imm;
        const struct IR_mipsOpdMem_t mem;
        const struct IR_mipsOpdRep_t rep;
        const struct IR_mipsOpdLab_t lab;
    } u;

    const struct IR_mipsClosure_t * cls;
};

LIST_DEFINE (IR_mipsOpdList, IR_mipsOpd)
LIST_CONST_DEFINE (IR_MipsOpdList, IR_mipsOpdList, IR_mipsOpd)

static inline IR_mipsOpd IR_MipsOpdImmInt32 (int32_t value)
{
    U_Create (IR_mipsOpd, r)
    {
        .kind = IR_mipsOpdImmKind,
        .u.imm = (struct IR_mipsOpdImm_t)
        {
            .kind = IR_mipsImmInt32Kind,
            .u.ival = value
        }
    };
    return r;
}

static inline IR_mipsOpd IR_MipsOpdImmUInt32 (uint32_t value)
{
    U_Create (IR_mipsOpd, r)
    {
        .kind = IR_mipsOpdImmKind,
        .u.imm = (struct IR_mipsOpdImm_t)
        {
            .kind = IR_mipsImmUInt32Kind,
            .u.uval = value
        }
    };
    return r;
}

static inline IR_mipsOpd IR_MipsOpdMem (intmax_t offset, IR_mipsOpd base)
{
    assert (base->kind == IR_mipsOpdRepKind);
    assert (base->u.rep.kind == IR_mipsOpdRepSrcKind);

    U_Create (IR_mipsOpd, r)
    {
        .kind = IR_mipsOpdMemKind,
        .u.mem = (struct IR_mipsOpdMem_t)
        {
            .offset = offset,
            .base = base
        }
    };
    return r;
}

/*
 * Label operand MUST NOT be used with branch or jump instructions since it WON'T be used for data
 * flow analysis which will lead to incorrect register allocation and very subtle bugs.
 */
static inline IR_mipsOpd IR_MipsOpdLab (Temp_label label)
{
    U_Create (IR_mipsOpd, r)
    {
        .kind = IR_mipsOpdLabKind,
        .u.lab = (struct IR_mipsOpdLab_t)
        {
            .label = label
        }
    };
    return r;
}

static inline IR_mipsOpd IR_MipsOpdRepExp (Temp_temp tmp, bool dst, IR_mipsClosure cls)
{
    U_Create (IR_mipsOpd, r)
    {
        .kind = IR_mipsOpdRepKind,
        .u.rep = (struct IR_mipsOpdRep_t)
        {
            .kind = dst ? IR_mipsOpdRepDstKind : IR_mipsOpdRepSrcKind,
            .u.tmp = tmp
        },

        .cls = cls
    };
    return r;
}

/*
 * Label operand MUST be used with branch or jump instruction since it WILL BE used with data flow
 * analysis.
 */
static inline IR_mipsOpd IR_MipsOpdRepJmp (Temp_label lab)
{
    U_Create (IR_mipsOpd, r)
    {
        .kind = IR_mipsOpdRepKind,
        .u.rep = (struct IR_mipsOpdRep_t)
        {
            .kind = IR_mipsOpdRepJmpKind,
            .u.lab = lab
        }
    };
    return r;
}

/****************
*  Statements  *
****************/

typedef struct IR_mipsOpc_t
{
    const struct M_opCode_t * spec;
    const struct IR_mipsOpdList_t * opdl;
} * IR_mipsOpc;

typedef struct IR_mipsLab_t
{
    Temp_label label;
} * IR_mipsLab;

typedef enum
{
    IR_mipsStmOpcKind,
    IR_mipsStmLabKind,
} IR_mipsStm_k;

typedef struct IR_mipsStm_t
{
    IR_mipsStm_k kind;
    union
    {
        const struct IR_mipsOpc_t opc;
        const struct IR_mipsLab_t lab;
    } u;
} * IR_mipsStm;

static inline IR_mipsStm IR_MipsStmOpc (
    const struct M_opCode_t * spec,
    const struct IR_mipsOpdList_t * opdl)
{
    U_Create (IR_mipsStm, r)
    {
        .kind = IR_mipsStmOpcKind,
        .u.opc = (struct IR_mipsOpc_t)
        {
            .spec = spec,
            .opdl = opdl,
        }
    };

    return r;
}

static inline IR_mipsStm IR_MipsStmLab (Temp_label label)
{
    U_Create (IR_mipsStm, r)
    {
        .kind = IR_mipsStmLabKind,
        .u.lab = (struct IR_mipsLab_t)
        {
            .label = label
        }
    };
    return r;
}

LIST_DEFINE (IR_mipsStmList, IR_mipsStm)
LIST_CONST_DEFINE (IR_MipsStmList, IR_mipsStmList, IR_mipsStm)
