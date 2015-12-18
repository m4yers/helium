#include <assert.h>

#include "ext/bool.h"
#include "ext/list.h"
#include "ext/util.h"
#include "ext/mem.h"

#define LOG_LEVEL LOG_LEVEL_WARNING
#include "ext/log.h"

#include "mipsmachine.h"
#include "frame.h"
#include "codegen.h"
#include "temp.h"

static Temp_tempList sink = NULL;

Temp_map F_tempMap;


/***********
 *  Frame  *
 ***********/

struct F_frame_
{
    /**
     * The name of the function
     */
    Temp_label name;

    /*
     * Return label used to jump from the middle of the function bock.
     */
    Temp_label ret;

    /**
     *
     * The formal parameters used to call this function. The space for these is reserved by the
     * caller and is not really a part of this stack frame. These a subject for escaping and all
     * formal parameters that do escape MUST be pushed to stack upon entering this frame.
     */
    F_accessList formals;
    int formalsNum;

    /**
     * The local variables of word size defined inside the function scope. These are subject for
     * escaping and alllocal variables that do escape MUST be pushed to stack after use and poped
     * back before use.
     */
    F_accessList words;

    /*
     * The local arrays of words, these are not used directly by the translator but referenced
     * through a special local variable called handle which is of size word and sits in words
     * list.
     */
    F_accessList arrays;

    /*
     * The list of virtual access points, their types and pottential storage(stack or register)
     * is unknown, but MUST be specified before frame lowering.
     */
    F_accessList virtuals;

    /*
     * Total word count for local data including one-word and multi-word allocations.
     */
    int wordsNum;

    /**
     * Maximum number of outgoing arguments used for calling other functions, since MIPS calling
     * convention specifies reserving 4 arguments space for every call this value cannot be less
     * than 4 for non-leaf function, the actual values are passed in registers a0-a4.
     */
    int outgoingMaxNum;

    /*
     * Number of t0-9 colors(registers) used for the current frame's code fragment. We HAVE to
     * allocate space for these registers on stack in case we call a function from our code
     * fragment.
     */
    int tUsed;

    /*
     * Number of t0-9 colors(registers) used for the current frame's code fragment. We have to
     * save these register before function execution(epilogue) and restore right before function
     * return(prologue).
     */

    int sUsed;

    /*
     * TODO
     * The flag indicates whether the frame's code fragment is a leaf function, if this is the
     * case we do not have to allocate any space for temp registers(t0-9) and any other register
     * we must be saved and restored because of function call.
     */
    bool isLeaf;
};

struct F_access_
{
    enum
    {
        FA_reg,          // stored in register if possible
        FA_stackWord,    // stored on stack
        FA_stackArray,   // stored on stack as an array of words
        FA_virtual,      // the type of storage and its size is unknown
    } kind;

    /*
     * If not NULL this current access point is a handle and points to this access points which
     * is usually an array of words.
     */
    F_access access;

    union
    {
        struct
        {
            /*
             * When somebody requests the access point it usually resolved as a register or stack
             * offset, virtual are special in this regard, they will return a container that is
             * dummy T_Eseq instance, the client code can do whatever necessary with it, and when
             * the actual type and storage of the access point is resolved the dummies will be
             * filled with register temp instance or stack offset.
             *
             * .stm = T_NoOp
             * .exp = FramePtr
             */
            T_expList containers;
        } virtual;
        /*
         * Temp register that might be assigned a real register by register allocator
         */
        Temp_temp reg;

        struct
        {
            /*
             * offset in bytes
             */
            int offset;
        } stackWord;

        struct
        {
            /*
             * offset in bytes
             */
            int offset;

            /*
             * Size of the array in words
             */
            int size;
        } stackArray;
    } u;
};

// TODO come up with a better way to print stuff
#define ACCESS_TOSTRING_BUFFER_SIZE 128
static char * Access_ToString (F_access access, char * buffer)
{
    if (!buffer)
    {
        buffer = checked_malloc (ACCESS_TOSTRING_BUFFER_SIZE);
    }

    char * current = buffer;

    current += sprintf (current, "Access %p ", access);
    switch (access->kind)
    {
    case FA_virtual:
    {
        current += sprintf (current, "kind: virtual");
        break;
    }
    case FA_reg:
    {
        current += sprintf (current, "kind: regWord, handles: %p",
                            access->access);
        break;
    }
    case FA_stackWord:
    {
        current += sprintf (current, "kind: stackWord, offset: %d, handles: %p",
                            access->u.stackWord.offset,
                            access->access);
        break;
    }
    case FA_stackArray:
    {
        current += sprintf (current, "kind: stackArray, offset: %d, size: %d",
                            access->u.stackArray.offset,
                            access->u.stackArray.size);
        break;
    }
    }

    return buffer;
}

#define FRAME_TOSTRING_BUFFER_SIZE 1024 * 10
static char * Frame_ToString (F_frame frame)
{
    char * buffer = checked_malloc (FRAME_TOSTRING_BUFFER_SIZE);
    char * current = buffer;
    current += sprintf (current, "Frame <%s> isLeaf: %s\n", frame->name->name,
                        frame->isLeaf ? "TRUE" : "FALSE");
    current += sprintf (current, "  outgoing: %d\n", frame->outgoingMaxNum);
    current += sprintf (current, "  tUsed: %d\n", frame->tUsed);
    current += sprintf (current, "  sUsed: %d\n", frame->sUsed);
    current += sprintf (current, "  formals:\n");
    LIST_FOREACH (access, frame->formals)
    {
        current += sprintf (current, "    %s\n", Access_ToString (access, NULL));
    }
    current += sprintf (current, "  words:\n");
    LIST_FOREACH (access, frame->words)
    {
        current += sprintf (current, "    %s\n", Access_ToString (access, NULL));
    }

    current += sprintf (current, "  arrays:\n");
    LIST_FOREACH (access, frame->arrays)
    {
        current += sprintf (current, "    %s\n", Access_ToString (access, NULL));
    }

    return buffer;
}

static F_access RegWordNew (Temp_temp reg, F_access access)
{
    F_access r = checked_malloc (sizeof (*r));
    r->kind = FA_reg;
    r->access = access;
    r->u.reg = reg;
    return r;
}

static F_access StackWordNew (int offset, F_access access)
{
    F_access r = checked_malloc (sizeof (*r));
    r->kind = FA_stackWord;
    r->access = access;
    r->u.stackWord.offset = offset;
    return r;
}

static F_access StackArrayNew (int offset, int size)
{
    F_access r = checked_malloc (sizeof (*r));
    r->kind = FA_stackArray;
    r->u.stackArray.offset = offset;
    r->u.stackArray.size = size;
    return r;
}

static F_access VirtualNew()
{
    F_access r = checked_malloc (sizeof (*r));
    r->kind = FA_virtual;
    return r;
}

F_frame F_NewFrame (Temp_label name, U_boolList formals)
{
    F_frame r = checked_malloc (sizeof (*r));
    r->name = name;
    r->ret = Temp_NewLabel();
    r->wordsNum = 0;
    r->words = NULL;
    r->formalsNum = 0;
    r->formals = NULL;
    // MIPS requires to allocate space at least for 4 arguments
    r->outgoingMaxNum = 4;
    r->tUsed = 0;
    r->sUsed = 1; // the 0th slot is already occupied by $fp

    if (formals && formals->head)
    {
        for (U_boolList l = formals; l; l = l->tail)
        {
            F_access access;
            // if escapes and within 4 arguments reg limit
            if (!l->head && r->formalsNum < 4)
            {
                Temp_temp temp = F_RegistersGet (regs_arguments, r->formalsNum);
                access = RegWordNew (temp, NULL);
            }
            else
            {
                access = StackWordNew (F_wordSize * r->formalsNum, NULL);
            }

            LIST_PUSH (r->formals, access);
            r->formalsNum++;
        }
    }

    DBG ("F_NewFrame %p:\n%s\n", r, Frame_ToString (r))

    return r;
}

Temp_label F_Name (F_frame frame)
{
    return frame->name;
}

Temp_label F_Ret (F_frame frame)
{
    return frame->ret;
}

F_accessList F_Formals (F_frame frame)
{
    return frame->formals;
}

F_access F_Alloc (F_frame frame, bool escape)
{
    F_access access;

    if (escape)
    {
        /*
         * since stack grows from HI to LO we reference local data with negative offset relative
         * to $fp
         */
        access = StackWordNew (-F_wordSize * ++frame->wordsNum, NULL);
    }
    else
    {
        access = RegWordNew (Temp_NewTemp(), NULL);
    }

    LIST_PUSH (frame->words, access);

    DBG ("F_Alloc %p for <%s>, escapes: %s\n",
         access,
         frame->name->name,
         BOOL_TO_STRING (escape))

    return access;
}

F_access F_AllocFrame (F_frame frame)
{
    return F_Alloc (frame, TRUE);
}

F_access F_AllocArray (F_frame frame, int words, bool escape)
{
    /*
     * updating the couter to the actual array size;
     */
    frame->wordsNum += words;

    /*
     * since stack grows from HI to LO we reference local data with negative offset relative
     * to $fp
     */
    F_access array = StackArrayNew (-F_wordSize * frame->wordsNum, words);
    LIST_PUSH (frame->arrays, array);

    F_access access;
    if (escape)
    {
        access = StackWordNew (-F_wordSize * ++frame->wordsNum, array);
    }
    else
    {
        access = RegWordNew (Temp_NewTemp(), array);
    }

    LIST_PUSH (frame->words, access);

    DBG ("F_AllocArray %p for <%s>, words: %d, handle: %p, escapes: %s\n",
         array,
         frame->name->name,
         words,
         access,
         BOOL_TO_STRING (escape))

    return access;
}

F_access F_AllocVirtual (F_frame frame)
{
    F_access v = VirtualNew();
    DBG ("F_AllocVirtual %p for <%s>\n", v, frame->name->name);
    LIST_PUSH (frame->virtuals, v);
    return v;
}

// FIXME code dupliation
F_access F_AllocMaterializeArray (F_frame frame, F_access access, int words, bool escape)
{
    assert (F_AllocIsVirtual (access));

    /*
     * updating the couter to the actual array size;
     */
    frame->wordsNum += words;

    /*
     * since stack grows from HI to LO we reference local data with negative offset relative
     * to $fp
     */
    F_access array = StackArrayNew (-F_wordSize * frame->wordsNum, words);
    LIST_PUSH (frame->arrays, array);

    T_expList containers = access->u.virtual.containers;

    if (escape)
    {
        access->kind = FA_stackWord;
        access->u.stackWord.offset = -F_wordSize * ++frame->wordsNum;
    }
    else
    {
        access->kind = FA_reg;
        access->u.reg = Temp_NewTemp();
    }

    LIST_FOREACH (c, containers)
    {
        c->u.ESEQ.exp = F_GetVar (access, c->u.ESEQ.exp);
    }

    access->access = array;
    LIST_REMOVE (frame->virtuals, access);
    LIST_PUSH (frame->words, access);

    DBG ("F_AllocMaterializeArray %p for <%s>, words: %d, handle: %p, escapes: %s\n",
         array,
         frame->name->name,
         words,
         access,
         BOOL_TO_STRING (escape));

    return access;

}

bool F_AllocIsVirtual (F_access access)
{
    assert (access);
    return access->kind = FA_virtual;
}

/***************
 *  Fragments  *
 ***************/

F_frag F_StringFrag (Temp_label label, const char * str)
{
    DBG ("F_StringFrag \"%s\", label: %s\n", str, label->name)
    F_frag r = checked_malloc (sizeof (*r));
    r->kind = F_stringFrag;
    r->u.str.label = label;
    r->u.str.str = str;
    return r;
}

F_frag F_ProcFrag (T_stm body, F_frame frame)
{
    DBG ("F_ProcFrag %p for <%s>\n", body, frame->name->name)
    F_frag r = checked_malloc (sizeof (*r));
    r->kind = F_procFrag;
    r->u.proc.body = body;
    r->u.proc.frame = frame;
    return r;
}

void F_ProcFunctionCall (F_frame frame, F_frame encolosing, T_expList args)
{
    frame->outgoingMaxNum = MAX (frame->outgoingMaxNum, T_ExpListLen (args));
    (void) encolosing;
}

T_stm F_ProcEntryExit1 (F_frame frame, T_stm stm)
{
    DBG ("F_ProcEntryExit1 for <%s>\n", frame->name->name)

    stm = T_Seq (T_Comment ("view-shift-end-"), stm);

    int count = 0;
    LIST_FOREACH (a, frame->formals)
    {
        if (a->kind == FA_stackWord)
        {
            stm = T_Seq (T_Move (
                             T_Mem (T_Binop (
                                        T_plus,
                                        T_Temp (F_FP()),
                                        T_Const (a->u.stackWord.offset))),
                             T_Temp (F_RegistersGet (regs_arguments, count))),
                         stm);
        }
        count++;
        if (count == 4)
        {
            break;
        }
    }

    stm = T_Seq (T_Comment ("view-shift-start-"), stm);
    stm = T_Seq (T_Label (frame->name), stm);

    return stm;
}

// TODO rename to F_PreRegAlloc
ASM_lineList F_ProcEntryExit2 (F_frame frame, ASM_lineList body)
{
    DBG ("F_ProcEntryExit2 for <%s>\n", frame->name->name)

    // Set local arrays' handles
    T_stmList stms = NULL;
    LIST_FOREACH (word, frame->words)
    {
        F_access access = word->access;
        if (access)
        {
            if (word->kind == FA_reg)
            {
                LIST_PUSH (stms, T_Move (
                               T_Temp (word->u.reg),
                               T_Binop (T_plus,
                                        T_Temp (fp),
                                        T_Const (access->u.stackArray.offset))));
            }
            else if (word->kind == FA_stackWord)
            {
                LIST_PUSH (stms, T_Move (
                               T_Mem (T_Binop (
                                          T_plus,
                                          T_Temp (fp),
                                          T_Const (word->u.stackWord.offset))),
                               T_Binop (T_plus,
                                        T_Temp (fp),
                                        T_Const (access->u.stackArray.offset))));
            }
            else
            {
                // WTF
                assert (0);
            }
        }
    }

    if (stms)
    {
        ASM_lineList handlers = F_CodeGen (frame, stms);
        LIST_INJECT (body, handlers, 1);
    }

    // HMM is the sink necessary here?
    return ASM_Splice (body, ASM_LineList (ASM_Oper ("", NULL, sink, NULL), NULL));
}

// TODO rename to F_PostRegAlloc
ASM_lineList F_ProcEntryExit3 (F_frame frame, ASM_lineList body, Temp_tempList colors)
{
    DBG ("F_ProcEntryExit3 for\n%s\n", Frame_ToString (frame))
    /*
     * Stack Frame:
     *
     * |          HI          |
     * +~~~~~~~~~~~~~~~~~~~~~~+
     * |                      |
     * |    previous frame    |
     * |                      |
     * +----------------------+
     * |  in arguments 5 - N  |
     * +----------------------+ < first 4 arguments passed in a0-a3
     * |  in arguments 1 - 4  |
     * +----------------------+ < $fp - start of the current stack frame or the end of the previous
     * |                      |
     * |    caller - saved    |
     * |       registers      |
     * |                      |
     * |          and         |
     * |                      |
     * |                      | used for data that requires more than machine word size,
     * |      local data      | e.g. arrays, records, large ints and reals etc.
     * |     (static area)    |
     * |                      |
     * |                      |
     * +----------------------+
     * |   optional padding   | to make local data 8-byte aligned
     * +----------------------+
     * |    return address    | < in case of leaf function
     * +----------------------+
     * |                      |
     * |    callee - saved    | stores register s0-s7 if used plus previous $fp which is s8
     * |       registers      |
     * |                      |
     * +----------------------+
     * |                      | TODO
     * |                      | dynamically grown area used for
     * |                      |     ( not yet implemented)
     * |     dynamic area     |
     * |                      |
     * |                      |
     * |                      |
     * +----------------------+ < outgoing arguments
     * | out arguments 5 - N  |
     * +----------------------+ < first 4 arguments passed in a0-a3
     * | out arguments 1 - 4  |
     * +----------------------+ < $sp - the end of the current statck frame, or start of the next
     * |                      |
     * |      next frame      |
     * |                      |
     * +~~~~~~~~~~~~~~~~~~~~~~+
     * |          LO          |
     */

    /*
     * Multiple results Stack Frame:
     *
     * +~~~~~~~~~~~~~~~~~~~~~~+
     * |                      |
     * |    previous frame    |
     * |                      |
     * +----------------------+
     * |  in arguments 5 - N  |
     * +----------------------+ < first 4 arguments passed in a0-a3
     * |  in arguments 1 - 4  |
     * +----------------------+ < $fp - start of the current stack frame or the end of the previous
     * |                      | TODO
     * |                      | space to store return values, this allows a caller to receive
     * |   out values 1 - N   | more than one result from a function call. If values 0 - 5 are
     * |                      | simple and can fit into a register they can be returned in v0-v1
     * |                      | and a0-a4
     * +----------------------+
     * |                      |
     * |                      | stores all local variables that require
     * |   local variables    | space on stack (escaping or spilled values)
     * |                      |
     * |                      |
     * +----------------------+
     * |   optional padding   | to make local data 8-byte aligned
     * +----------------------+
     * |    return address    | < in case of leaf function
     * +----------------------+
     * |                      |
     * |    callee - saved    | stores register s0-s7 if used plus previous $fp which is s8
     * |       registers      |
     * |                      |
     * +----------------------+
     * |                      | TODO
     * |                      | dynamically grown area used for
     * |                      |     ( not yet implemented)
     * |     dynamic area     |
     * |                      |
     * |                      |
     * |                      |
     * +----------------------+ < outgoing arguments
     * | out arguments 5 - N  |
     * +----------------------+ < first 4 arguments passed in a0-a3
     * | out arguments 1 - 4  |
     * +----------------------+ < $sp - the end of the current statck frame, or start of the next
     * |                      |
     * |      next frame      |
     * |                      |
     * +~~~~~~~~~~~~~~~~~~~~~~+
     */

    /*
     * Making sure we materialized all virtual allocations
     */
    size_t vlen = LIST_SIZE (frame->virtuals);
    assert (vlen == 0);

    /*
     * We need to allocate stack space for each color(register) we use and are caller-safe.
     * Thus we allocate space only for t0-9 registers; v0-1 supposed to be trashed by func
     * call and we do not save those; a0-3 have their space allocated already in the parent
     * stack frame. Registers s0-7 are saved by the callee.
     */
    LIST_FOREACH (color, colors)
    {
        if (F_RegistersContains (regs_caller_save, color))
        {
            frame->tUsed++;
        }
        else if (F_RegistersContains (regs_callee_save, color))
        {
            frame->sUsed++;
        }
    }

    // TEST remove this
    /* frame->tUsed = 10; */
    /* frame->sUsed = 5; */

    /* printf ("%s\n", Frame_ToString (frame)); */


    int lSize = 0;
    int tSize = 0;
    int ldssSize = 0;
    // FIXME rename to frame length and add make framWords be the actual size in words
    int frameSize = 0;

    /*
     * The size of Local Data Storage Section, this segment contains all local
     * escaping variables and temporary caller-save registers(t0-9) if used.
     */

    /*
     * Each escaping local variable goes into LDSS
     */
    /* LIST_FOREACH (access, frame->words) */
    /* { */
    /*     if (access->kind == FA_stackWord) */
    /*     { */
    /*         frameSize++; */
    /*     } */
    /* } */

    // words allocated for stack words and arrays
    frameSize += frame->wordsNum;

    lSize = frameSize * F_wordSize;

    /*
     * Each allocated color has its own place on stack
     */
    frameSize += frame->tUsed;
    tSize = frame->tUsed * F_wordSize;

    /*
     * Padding.
     * We need to make sure the Local Data Storage Section part is on 8-byte border
     */
    frameSize += (frameSize % 2 ? 1 : 0);

    ldssSize = frameSize * F_wordSize;
    /*
     * Return address
     */
    frameSize++;
    /*
     * Calee-save registers used it this frame's code
     */
    frameSize += frame->sUsed;
    /*
     * Maximum number of arguments this function will be using for calls
     */
    frameSize += frame->outgoingMaxNum;
    /*
     * Multiplying by word size to get real value
     */
    frameSize *= F_wordSize;

    T_stmList stms = NULL;

    /*
     * TODO regarding FP and RA
     *
     * Generally, this is a dumb work moving these to stack. You need to check for free t0-t9 or
     * a0-a3 registers since they are supposed to be trashed by the callee anyway and move the
     * old $fp and $ra there if the callee is a leaf. Before calling another func move those two
     * to free s0-s7, or even better move it there in the beginning if possible for non-leaf.
     *
     * if leaf  : $fp and $ra -> a0-a3 or t0-t9
     * non-leaf : $fp and $ra -> s0-s7
     *
     * in-module optimization: $ra is not changed by leaf functions
     *
     * Anyway, looks like a window for optimization.
     */

    LIST_PUSH (stms, T_Comment ("epilogue-"));

    // increase stack size
    LIST_PUSH (stms, T_Move (T_Temp (sp), T_Binop (T_minus, T_Temp (sp), T_Const (frameSize))));

    // save RA to stack
    LIST_PUSH (stms, T_Move (
                   T_Mem (
                       T_Binop (
                           T_minus,
                           T_Temp (sp),
                           T_Const (ldssSize + F_wordSize - frameSize))),
                   T_Temp (ra)));

    // Save Callee-Save Registesrs to stack including old $fp
    for (int i = 0; i < frame->sUsed; ++i)
    {
        LIST_PUSH (stms,
                   T_Move (
                       T_Mem (
                           T_Binop (
                               T_minus,
                               T_Temp (sp),
                               // 1 for ra and 1 for word offset
                               T_Const (ldssSize + F_wordSize + (i + 1) * F_wordSize - frameSize))),
                       T_Temp (F_RegistersGet (regs_callee_save, i))));
    }

    // Set FP
    LIST_PUSH (stms, T_Move (T_Temp (fp), T_Binop (T_plus, T_Temp (sp), T_Const (frameSize))));

    LIST_PUSH (stms, T_Comment ("body-"));

    ASM_lineList prologue = F_CodeGen (frame, stms);

    stms = NULL;

    LIST_PUSH (stms, T_Comment ("prologue-"));

    // all ret leads here
    LIST_PUSH (stms, T_Label (frame->ret));

    // Restore RA
    LIST_PUSH (stms, T_Move (
                   T_Temp (ra),
                   T_Mem (
                       T_Binop (
                           T_minus,
                           T_Temp (sp),
                           T_Const (ldssSize + F_wordSize - frameSize)))));

    // Restore CeSR to stack including old $fp
    for (int i = 0; i < frame->sUsed; ++i)
    {
        LIST_PUSH (stms,
                   T_Move (
                       T_Temp (F_RegistersGet (regs_callee_save, i)),
                       T_Mem (
                           T_Binop (
                               T_minus,
                               T_Temp (sp),
                               T_Const (ldssSize + F_wordSize + (i + 1) * F_wordSize - frameSize)))));
    }

    // Restore SP
    // HMM it was $sp <- $fp before, should have i kept it?
    LIST_PUSH (stms, T_Move (T_Temp (sp), T_Binop (T_plus, T_Temp (sp), T_Const (frameSize))));

    // Return
    LIST_PUSH (stms, T_Rjump (T_Temp (ra)));

    ASM_lineList epilogue = F_CodeGen (frame, stms);

    LIST_INJECT (body, prologue, 1);
    LIST_JOIN (body, epilogue);

    /* LIST_JOIN(epilogue, body); */
    /* LIST_JOIN(epilogue, prologue); */
    /* body = epilogue; */

    if (frame->tUsed != 0)
    {
        /*
         * Caller-Save registers saving/restoring around function calls
         * TODO clone the lists for every function call
         */
        stms = NULL;
        for (int i = 0; i < frame->tUsed; ++i)
        {
            LIST_PUSH (stms,
                       T_Move (
                           T_Mem (
                               T_Binop (
                                   T_minus,
                                   T_Temp (fp),
                                   T_Const (lSize + (i + 1) * F_wordSize))),
                           T_Temp (F_RegistersGet (regs_caller_save, i))));
        }
        ASM_lineList save = F_CodeGen (frame, stms);

        stms = NULL;
        for (int i = 0; i < frame->tUsed; ++i)
        {
            LIST_PUSH (stms,
                       T_Move (
                           T_Temp (F_RegistersGet (regs_caller_save, i)),
                           T_Mem (
                               T_Binop (
                                   T_minus,
                                   T_Temp (fp),
                                   T_Const (lSize + (i + 1) * F_wordSize)))));
        }

        ASM_lineList restore = F_CodeGen (frame, stms);

        for (ASM_lineList l = body, prev = NULL; l; prev = l, l = l->tail)
        {
            if (l->head->kind == I_META)
            {
                if (l->head->u.META.kind == I_META_CALL_IN)
                {
                    prev->tail = save;
                    LIST_JOIN (save, l->tail);
                }
                else if (l->head->u.META.kind == I_META_CALL_OUT)
                {
                    prev->tail = restore;
                    LIST_JOIN (restore, l->tail);
                }
            }
        }
    }

    return body;
}

/**********
 *  Main  *
 **********/

void F_Init()
{
    LIST_PUSH (sink, zero);
    LIST_PUSH (sink, ra);
    LIST_PUSH (sink, sp);
    LIST_JOIN (sink, regs_callee_save->temps);

    F_tempMap = Temp_Empty();

    F_RegistersToMap (F_tempMap, regs_special);
}

Temp_temp F_Zero (void)
{
    return zero;
}

Temp_temp F_SP (void)
{
    return sp;
}

Temp_temp F_FP (void)
{
    return fp;
}

Temp_temp F_RA (void)
{
    return ra;
}

Temp_temp F_RV (void)
{
    return v0;
}

T_exp F_GetVar (F_access access, T_exp framePtr)
{
    if (access->kind == FA_stackWord)
    {
        // TODO factor out this out of here somehow
        return T_Mem (T_Binop (T_plus, framePtr, T_Const (access->u.stackWord.offset)));
    }
    else if (access->kind == FA_reg)
    {
        /*
         * FIXME this is a bit tricky, when translation asks for the actual access location
         * inside the frame it might give the function not frame pointer but some offset from it
         * if the access belongs to a different scope, but if something went wrong with escape
         * analysis the access type can be a reg which is ALWAYS a single T_Temp instance then all
         * scope resolving done by translation will be LOST.
         */
        assert (framePtr->kind == T_TEMP
                && framePtr->u.TEMP == fp
                && "The access point is a reg but the base is not $fp");
        return T_Temp (access->u.reg);
    }
    else if (access->kind == FA_virtual)
    {
        T_exp dummy = T_Eseq (T_NoOp(), framePtr);
        LIST_PUSH (access->u.virtual.containers, dummy);
        return dummy;
    }
    else
    {
        assert (0);
    }
}

T_exp F_ExternalCall (const char * name, T_expList args)
{
    return T_Call (T_Name (Temp_NamedLabel (name)), args);
}
