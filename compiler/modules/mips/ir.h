#pragma once

#include <stdint.h>

#include "util/list.h"
#include "util/bool.h"
#include "util/util.h"

#include "core/machine.h"
#include "core/temp.h"
#include "core/ir.h"

/**************
*  Operands  *
**************/

typedef struct IR_mipsOpdImm_t
{
    const bool is_signed;
    union
    {
        const uintmax_t uval;
        const intmax_t ival;
    } u;
} * IR_mipsOpdImm;

static inline IR_mipsOpdImm IR_MipsOpdImmInt (intmax_t value)
{
    U_Create (IR_mipsOpdImm, r)
    {
        .is_signed = TRUE,
         .u.ival = value
    };
    return r;
}

static inline IR_mipsOpdImm IR_MipsOpdImmUInt (uintmax_t value)
{
    U_Create (IR_mipsOpdImm, r)
    {
        .is_signed = FALSE,
         .u.uval = value
    };
    return r;
}

typedef struct IR_mipsOpdLab_t
{
    Temp_label label;
} * IR_mipsOpdLab;

static inline IR_mipsOpdLab IR_MipsOpdLab (Temp_label label)
{
    U_Create (IR_mipsOpdLab, r)
    {
        .label = label
    };
    return r;
}

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
        const struct T_exp_t * dst;
        const struct T_exp_t * src;
        const struct Temp_label_t * jmp;
    } u;
} * IR_mipsOpdRep;

static inline IR_mipsOpdRep IR_MipsOpdRepDst (const struct T_exp_t * dst)
{
    U_Create (IR_mipsOpdRep, r)
    {
        .kind = IR_mipsOpdRepDstKind,
         .u.dst = dst
    };
    return r;
}

static inline IR_mipsOpdRep IR_MipsOpdRepSrc (const struct T_exp_t * src)
{
    U_Create (IR_mipsOpdRep, r)
    {
        .kind = IR_mipsOpdRepSrcKind,
         .u.src = src
    };
    return r;
}

static inline IR_mipsOpdRep IR_MipsOpdRepJmp (const struct Temp_label_t * jmp)
{
    U_Create (IR_mipsOpdRep, r)
    {
        .kind = IR_mipsOpdRepJmpKind,
         .u.jmp = jmp
    };
    return r;
}

typedef enum
{
    IR_mipsOpdImmKind,
    IR_mipsOpdLabKind,
    IR_mipsOpdRepKind
} IR_mipsOpd_k;

typedef struct IR_mipsOpd_t
{
    IR_mipsOpd_k kind;

    union
    {
        const struct IR_mipsOpdImm_t imm;
        const struct IR_mipsOpdRep_t rep;
        const struct IR_mipsOpdLab_t lab;
    } u;
} * IR_mipsOpd;

LIST_DEFINE (IR_mipsOpdList, IR_mipsOpd)
LIST_CONST_DEFINE (IR_MipsOpdList, IR_mipsOpdList, IR_mipsOpd)

static inline IR_mipsOpd IR_MipsOpImm (const struct IR_mipsOpdImm_t imm)
{
    U_Create (IR_mipsOpd, r)
    {
        .kind = IR_mipsOpdImmKind,
         .u.imm = imm
    };
    return r;
}

static inline IR_mipsOpd IR_MipsOpRep (const struct IR_mipsOpdRep_t rep)
{
    U_Create (IR_mipsOpd, r)
    {
        .kind = IR_mipsOpdRepKind,
         .u.rep = rep
    };
    return r;
}

static inline IR_mipsOpd IR_MipsOpLab (const struct IR_mipsOpdLab_t lab)
{
    U_Create (IR_mipsOpd, r)
    {
        .kind = IR_mipsOpdLabKind,
         .u.lab = lab
    };
    return r;
}

/****************
*  Statements  *
****************/

typedef struct IR_mipsOpc_t
{
    const struct M_opCode_t spec;
    const struct IR_mipsOpdList_t opds;
    const struct T_stmList_t pre;
    const struct T_stmList_t post;
} * IR_mipsOpc;

static inline IR_mipsOpc IR_MipsOpc (
    const struct M_opCode_t spec,
    const struct IR_mipsOpdList_t opds,
    const struct T_stmList_t pre,
    const struct T_stmList_t post)
{
    U_Create (IR_mipsOpc, r)
    {
        .spec = spec,
         .opds = opds,
          .pre = pre,
           .post = post
    };

    return r;
}

typedef struct IR_mipsLab_t
{
    Temp_label label;
} * IR_mipsLab;

static inline IR_mipsLab IR_MipsLab (Temp_label label)
{
    U_Create (IR_mipsLab, r)
    {
        .label = label
    };
    return r;
}

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

static inline IR_mipsStm IR_MipsStmOpc (const struct IR_mipsOpc_t opc)
{
    U_Create (IR_mipsStm, r)
    {
        .kind = IR_mipsStmOpcKind,
         .u.opc = opc
    };
    return r;
}

LIST_DEFINE (IR_mipsStmList, IR_mipsStm)
LIST_CONST_DEFINE (IR_MipsStmList, IR_mipsStmList, IR_mipsStm)
