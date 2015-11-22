#ifndef TYPES_H_K4WAGU3S
#define TYPES_H_K4WAGU3S

#include <symbol.h>

typedef struct Ty_ty_ * Ty_ty;
typedef struct Ty_tyList_ * Ty_tyList;
typedef struct Ty_field_ * Ty_field;
typedef struct Ty_fieldList_ * Ty_fieldList;

struct Ty_ty_
{
    /**
     *
     * The kind part qualifies the type.
     *
     * TODO give a proper explanation on difference between Invalid and Unknown and how
     * to use them
     *
     * @Ty_invalid and @Ty_unknown are non-language types used to provide correct error
     * information outside the semantic module.
     */
    enum
    {
        Ty_auto,
        Ty_invalid, Ty_unknown,
        Ty_nil, Ty_void,
        Ty_int, Ty_string,
        Ty_array, Ty_record,
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

struct Ty_tyList_
{
    Ty_ty head;
    Ty_tyList tail;
};
struct Ty_field_
{
    S_symbol name;
    Ty_ty ty;
};
struct Ty_fieldList_
{
    Ty_field head;
    Ty_fieldList tail;
};

Ty_ty Ty_Auto (void);
Ty_ty Ty_Invalid (void);
Ty_ty Ty_Nil (void);
Ty_ty Ty_Int (void);
Ty_ty Ty_String (void);
Ty_ty Ty_Void (void);

Ty_ty Ty_Unknown (const char * name);
Ty_ty Ty_Record (Ty_fieldList fields);
Ty_ty Ty_Array (Ty_ty type, int size);
Ty_ty Ty_Name (S_symbol sym, Ty_ty ty);

Ty_tyList Ty_TyList (Ty_ty head, Ty_tyList tail);
Ty_field Ty_Field (S_symbol name, Ty_ty ty);
Ty_fieldList Ty_FieldList (Ty_field head, Ty_fieldList tail);

int Ty_SizeOf (Ty_ty type);

void Ty_print (Ty_ty t);
void TyList_print (Ty_tyList list);

#endif /* end of include guard: TYPES_H_K4WAGU3S */
