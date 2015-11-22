#ifndef ASSEM_H_GFKKRYV7
#define ASSEM_H_GFKKRYV7

#include "temp.h"

typedef struct
{
    Temp_labelList labels;
} * ASM_targets;

ASM_targets ASM_Targets (Temp_labelList labels);

typedef struct ASM_line_ * ASM_line;
struct ASM_line_
{
    enum {I_OPER, I_LABEL, I_MOVE, I_META} kind;
    union
    {
        struct
        {
            const char * assem;
            Temp_tempList dst, src;
            ASM_targets jumps;
        } OPER;
        struct
        {
            const char * assem;
            Temp_label label;
        } LABEL;
        struct
        {
            const char * assem;
            Temp_tempList dst, src;
        } MOVE;
        struct
        {
            enum  { I_META_CALL_IN, I_META_CALL_OUT, I_META_COMMENT } kind;
            const char * COMMENT;
        } META;
    } u;
};

typedef struct ASM_lineList_ * ASM_lineList;
struct ASM_lineList_
{
    ASM_line head;
    ASM_lineList tail;
};

ASM_line ASM_Oper (const char * a, Temp_tempList d, Temp_tempList s, ASM_targets j);
ASM_line ASM_Label (const char * a, Temp_label label);
ASM_line ASM_Move (const char * a, Temp_tempList d, Temp_tempList s);

// HMM... not sure about call meta, is it possible not to use them
ASM_line ASM_MetaCallIn (void);
ASM_line ASM_MetaCallOut (void);
ASM_line ASM_MetaCallComment (const char * text);

ASM_lineList ASM_LineList (ASM_line head, ASM_lineList tail);

ASM_lineList ASM_Splice (ASM_lineList a, ASM_lineList b);

char * ASM_Format (
    char * result,
    const char * assem,
    Temp_tempList dst,
    Temp_tempList src,
    ASM_targets jumps,
    Temp_map m);

char * ASM_LineToString (ASM_line l, Temp_map m);
char * ASM_LineListToString (ASM_lineList list, Temp_map m);
void ASM_PrintLine (FILE * out, ASM_line i, Temp_map m);

#endif /* end of include guard: ASSEM_H_GFKKRYV7 */
