#ifndef TYPES_H_K4WAGU3S
#define TYPES_H_K4WAGU3S

#include "ext/str.h"

#include "symbol.h"

typedef struct Ty_ty_ * Ty_ty;
typedef struct Ty_field_ * Ty_field;

LIST_DEFINE (Ty_tyList, Ty_ty)
LIST_DEFINE (Ty_fieldList, Ty_field)

struct Ty_ty_
{
    enum
    {
        Ty_auto, Ty_invalid,
        Ty_nil, Ty_void,
        Ty_int, Ty_pointer,
        Ty_array, Ty_record,
        Ty_string,
        Ty_name
    } kind;

    /**
     *
     * The meta part describes the type aside from its properties.
     *
     * @name is the name it has been assigned to upon creation, it will remain the same even
     * for cycle type declarations. For generality sake it cannot be a S_Symbol instance
     */
    struct
    {
        const char * name;
        bool is_internal;
        bool is_pointer;
        bool is_handle;
    } meta;

    union
    {
        Ty_fieldList record;
        struct
        {
            Ty_ty type;
            int size;
        } array;
        struct
        {
            S_symbol sym;
            Ty_ty ty;
        } name;
    } u;


};

struct Ty_field_
{
    S_symbol name;
    Ty_ty ty;
};

Ty_ty Ty_Auto (void);
Ty_ty Ty_Invalid (void);
Ty_ty Ty_Nil (void);
Ty_ty Ty_Int (void);
Ty_ty Ty_String (void);
Ty_ty Ty_Void (void);

Ty_ty Ty_Record (Ty_fieldList fields);
Ty_ty Ty_Array (Ty_ty type, int size);
Ty_ty Ty_Name (S_symbol sym, Ty_ty ty);

Ty_tyList Ty_TyList (Ty_ty head, Ty_tyList tail);
Ty_field Ty_Field (S_symbol name, Ty_ty ty);
Ty_fieldList Ty_FieldList (Ty_field head, Ty_fieldList tail);

Ty_ty GetActualType (Ty_ty ty);
String GetQTypeName (Ty_ty ty, String str);
int Ty_SizeOf (Ty_ty type);

#endif /* end of include guard: TYPES_H_K4WAGU3S */
