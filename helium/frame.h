#ifndef FRAME_H_R4HCSNDG
#define FRAME_H_R4HCSNDG

#include "ext/list.h"

#include "tree.h"
#include "temp.h"
#include "asm.h"

/***********
 *  Frame  *
 ***********/

typedef struct F_frame_ * F_frame;

typedef struct F_access_ * F_access;

LIST_DEFINE (F_accessList, F_access)

F_frame F_NewFrame (Temp_label name, U_boolList formals);
F_accessList F_Formals (F_frame frame);
Temp_label F_Name (F_frame frame);
Temp_label F_Ret (F_frame frame);

/**
 * Allocates word size variable
 *
 * @frame Current stack frame
 * @escape If TRUE the variable will be stored on stack and pushed to a register before usage,
 * otherwise the variable will reside in a register if there is one free at the moment and pushed
 * to stack only if spilled.
 */
F_access F_Alloc (F_frame frame, bool escape);

/**
 * Allocates word size variable stored on stack(escapes by default)
 *
 * @frame Current stack frame
 */
F_access F_AllocFrame (F_frame frame);

/**
 * The method allocates specified number of bytes and returns handle(pointer) to this stack based
 * memory block. E.g. allocating 4 word size array
 *
 * |          HI          |
 * +~~~~~~~~~~~~~~~~~~~~~~+ > local data area start
 * |                      |
 * +----------------------+
 * |        Word 4        |
 * +----------------------+
 * |        Word 3        |
 * +----------------------+
 * |        Word 2        |
 * +----------------------+
 * |        Word 1        |
 * +----------------------+ < you receive an access point that contains address of the first word
 * |                      |
 * |                      |
 * +~~~~~~~~~~~~~~~~~~~~~~+ > local data area end
 * |          LO          |
 *
 * Then to reference a particular word or set of words you calculate the offset and add it to the
 * base handle address or do whatever your machine addressing mode allows.
 *
 * NOTE: this is the method to use when the type size exceeds native word size.
 *
 * @frame Current stack frame object
 * @words Size in words of space you want to reserve
 * @escape If TRUE the handle to mem block will behave as escaping var(i.e. pushed to stack frame
 * and poped before use; if FALSE it will be kept in a register. Normally you want it be in a reg
 * all the time, since concept of escaping is of no real use here.
 *
 * HMM still not sure about escaping here
 * TODO what about alignment?
 */
F_access F_AllocArray (F_frame frame, int words, bool escape);

F_access F_AllocVirtual (F_frame frame);
F_access F_AllocMaterializeArray (F_frame frame, F_access access, int words, bool escape);

bool F_AllocIsVirtual (F_access access);

/***************
 *  Fragments  *
 ***************/

typedef struct F_frag_ * F_frag;
struct F_frag_
{
    enum
    {
        F_stringFrag,
        F_procFrag
    } kind;

    union
    {
        struct
        {
            Temp_label label;
            const char * str;
        } str;

        struct
        {
            T_stm body;
            F_frame frame;
        } proc;

    } u;
};

F_frag F_StringFrag (Temp_label label, const char * str);
F_frag F_ProcFrag (T_stm body, F_frame frame);

LIST_DEFINE (F_fragList, F_frag)

/*
 * Gets necessary information(number of arguments) out of function call and stores it to
 * the frame structure.
 */
void F_ProcFunctionCall (F_frame frame, F_frame encolosing,  T_expList args);
T_stm        F_ProcEntryExit1 (F_frame frame, T_stm stm);
ASM_lineList F_ProcEntryExit2 (F_frame frame, ASM_lineList body);
ASM_lineList F_ProcEntryExit3 (F_frame frame, ASM_lineList body, Temp_tempList colors);

/**********
 *  Main  *
 **********/

extern const int F_wordSize;

extern Temp_map F_tempMap;
Temp_temp F_Zero (void);
Temp_temp F_SP (void);
Temp_temp F_FP (void);
Temp_temp F_RA (void);
Temp_temp F_RV (void);

T_exp F_GetVar (F_access access, T_exp framePtr);
T_exp F_ExternalCall (const char * name, T_expList args);

void F_Init (void);

#endif /* end of include guard: FRAME_H_R4HCSNDG */
