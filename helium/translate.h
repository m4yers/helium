#ifndef TRANSLATE_H_ZDIHRPIG
#define TRANSLATE_H_ZDIHRPIG

#include "ast_helium.h"
#include "temp.h"
#include "frame.h"
#include "types.h"
#include "util.h"
#include "program.h"

// forward declaration
struct Sema_Context_t;

typedef struct Tr_exp_ * Tr_exp;
typedef struct Tr_access_ * Tr_access;

typedef struct Tr_accessList_ * Tr_accessList;
typedef struct Tr_level_ * Tr_level;

struct Tr_level_
{
    Temp_label name;
    Tr_level parent;
    Tr_accessList formals;
    Tr_accessList locals;
    F_frame frame;
};

struct Tr_accessList_
{
    Tr_access head;
    Tr_accessList tail;
};

typedef struct Tr_expList_ * Tr_expList;
struct Tr_expList_
{
    Tr_exp head;
    Tr_expList tail;
};

Tr_expList Tr_ExpList (Tr_exp head, Tr_expList tail);

void Tr_Init (struct Sema_Context_t * c);
void Tr_ProcEntryExit (struct Sema_Context_t * c, Tr_level level, Tr_exp body);
void Tr_AddCodeFragment(struct Sema_Context_t * c, Tr_exp fragment);

/***************
 *  Variables  *
 ***************/

Tr_exp Tr_SimpleVar (Tr_access access, Tr_level level, bool deref);
Tr_exp Tr_FieldVar (Tr_exp var, Ty_ty type, S_symbol field, bool deref);
Tr_exp Tr_SubscriptVar (Tr_exp var, Ty_ty type, Tr_exp subscript, bool deref);

/*****************
 *  Expressions  *
 *****************/

Tr_exp Tr_DerefExp (Tr_exp exp);
Tr_exp Tr_Seq (Tr_exp seq, Tr_exp current);
Tr_exp Tr_Call (Temp_label label, Tr_level encolosing, Tr_level own, Tr_expList args);
Tr_exp Tr_Nil (void);
Tr_exp Tr_Void (void);
Tr_exp Tr_Int (int value);
Tr_exp Tr_String (struct Sema_Context_t * c, const char * value);
Tr_exp Tr_Op (A_oper op, Tr_exp left, Tr_exp right, Ty_ty ty);
Tr_exp Tr_ArrayExp (Tr_exp base, Ty_ty type, Tr_expList list, int offset);
Tr_exp Tr_RecordExp (Tr_exp base, Ty_ty type, Tr_expList list, int offset);
Tr_exp Tr_Assign (Tr_exp left, Tr_exp right);
Tr_exp Tr_Memcpy (Tr_exp dst, Tr_exp src, size_t words);
Tr_exp Tr_If (Tr_exp test, Tr_exp te, Tr_exp fe);
Tr_exp Tr_While (Tr_exp test, Tr_exp body, Temp_label done);
Tr_exp Tr_For (Tr_exp lo, Tr_exp hi, Tr_exp body, Tr_access iter, Temp_label done);
Tr_exp Tr_Break (Temp_label done);
Tr_exp Tr_Ret (Tr_level level, Tr_exp exp);
Tr_exp Tr_Exit (Tr_exp exp);
Tr_exp Tr_Asm (A_asmStmList stms, Temp_tempList dst, Temp_tempList src);
Tr_exp Tr_AsmOld (const char * code, Tr_exp data, U_stringList dst, U_stringList src);

/**************
 *  Whatever  *
 **************/

Tr_level Tr_NewLevel (Tr_level parent, Temp_label name, U_boolList formals);

Temp_label Tr_ScopedLabel (Tr_level level, const char * name);

Tr_accessList Tr_Formals (Tr_level level);

Tr_access Tr_AllocVirtual (Tr_level level, S_symbol name);
Tr_access Tr_AllocMaterialize (Tr_access access, Tr_level level, Ty_ty type, bool escape);
Tr_access Tr_Alloc (Tr_level level, Ty_ty type, S_symbol name, bool escape);
void Tr_AllocDelete(Tr_level level, Tr_access access);

#endif /* end of include guard: TRANSLATE_H_ZDIHRPIG */
