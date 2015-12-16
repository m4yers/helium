#ifndef STRING_H_A4HVP9OX
#define STRING_H_A4HVP9OX

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "ext/bool.h"

typedef struct String_t * String;

struct String_t
{
    char * data;

    /*
     * If True the String instance references the data that was passed to the Init method. If
     * False the data was copied to a newly allocated memory block.
     */
    bool is_static;

    /*
     * The size of the storage space currently allocated for the string, expressed in terms of
     * characters.
     */
    size_t capacity;

    /*
     * This is the number of actual objects held in the string, which is not necessarily equal
     * to its storage capacity.
     */
    size_t size;
};

/*
 * struct String_t initialization expression. Just a shortcut.
 */
#define String(s)\
    { .data = (char *) s, .is_static = TRUE, .capacity = strlen(s) + 1, .size = strlen(s) }

/*
 * Initializes the String instance with raw c-like string. By default the instance of String
 * WILL NOT COPY passed sequence but will reference on every read-only request. If you attempt
 * to modify String in any way(append, insert etc.) the sequence will be copied into a newly
 * allocated memory block.
 */
String String_Init (String s, const char * data);

/*
 * Frees all memory allocated for the string except the instance istself assuming it was allocated
 * on stack.
 */
void String_Fini (String s);

/*
 * Same as String_Init but the instance is allocated on heap
 */
String String_New (const char * data);

/*
 * Same as String_Fini plus the instance is freed and assigned to NULL.
 */
#define String_Delete(s) String_Fini(s); free(s); s = NULL;

/*
 * If the String instance references static string this method will force it to allocate new
 * memory block and copy the string there.
 */
void String_NoStatic (String s);

/*
 * Checks whether String instance references static string.
 */
bool String_IsStatic (String s);

/*
 * Returns whether the string is empty (i.e. whether its size is 0). This function does not
 * modify the container in any way. To clear the content of a string, see String_Clear.
 */
bool String_Empty (const String s);

/*
 * Erases the contents of the string, which becomes an empty string (with a length of 0 characters)
 */
void String_Clear (String s);

/*
 * Requests that the string capacity be at least enough to contain n chars.
 *
 *
 * If n is greater than the current string capacity, the function causes the container to
 * reallocate its storage increasing its capacity to n (or greater).
 *
 * In all other cases, the function call does not cause a reallocation and the string capacity
 * is not affected.
 *
 * This function has no effect on the string size and cannot alter its characters.
 */
void String_Reserve (String s, size_t n);

/*
 * Resizes the container so that it contains n characters.
 *
 * If n is smaller than the current container size, the content is reduced to its first n
 * characters. The actual data won't be erased unless overwritten by subsequent calls to string
 * functions. Be cautions though, by truncating the string using this method you will lose access
 * to your data, this might lead to memory leaks.
 *
 * If n is greater than the current container size, the content is expanded by inserting at
 * the end as many characters as needed to reach a size of n. If val is specified, the new characters
 * are initialized as copies of val, otherwise, they are value-initialized.
 *
 * If n is also greater than the current container capacity, an automatic reallocation of the
 * allocated storage space takes place.
 */
void String_Resize (String s, size_t n, char c);

int String_Diff_s (String a, String b);
int String_Diff_cp (String a, const char * b);

/*
 * Compares String and the second operand including nul charactesrs and returns -1 if two String
 * instances contain equal sequence of characters. If the two String instances are different the
 * index of the first unmatched character is returned.
 */
#define String_Diff(a,b)                                             \
    _Generic ((0,b),                                                 \
            char *: String_Diff_cp,                                  \
            const char *: String_Diff_cp,                            \
            struct String_t *: String_Diff_s                         \
            )(a,b)

bool String_Equal_s (String a, String b);
bool String_Equal_c (String a, const char * b);

/*
 * Returns True if a c char sequance is equal to the one in the String instance
 */
#define String_Equal(a,b)                                            \
    _Generic ((0,b),                                                 \
            char *: String_Equal_c,                                  \
            const char *: String_Equal_c,                            \
            struct String_t *: String_Equal_s                        \
            )(a,b)

void String_Assign_s (String s, String o);
void String_Assign_cp (String s, const char * c);

/*
 * Assigns a new value to the string, replacing its current contents. The resulting String is not
 * static.
 */
#define String_Assign(a,b)                                           \
    _Generic ((0,b),                                                 \
            char *: String_Assign_cp,                                \
            const char *: String_Assign_cp,                          \
            struct String_t *: String_Assign_s                       \
            )(a,b)

void String_Insert_s (String s, size_t pos, String o);
void String_Insert_cp (String s, size_t pos, const char * c);

/*
 * Inserts additional characters into the string right before the character indicated by position.
 */
#define String_Insert(s,pos,o)                                       \
    _Generic ((0,o),                                                 \
            char *: String_Insert_cp,                                \
            const char *: String_Insert_cp,                          \
            struct String_t *: String_Insert_s                       \
            )(s,pos,o)

/*
 * Calls strcmp on Strings content
 */
int String_Cmp (String a, String b);

/*
 * Appends character c to the end of the string, increasing its length by one.
 */
void String_PushBack (String s, char c);

/*
 * Removes the last character in the string, effectively reducing the container size by one.
 *
 * This destroys the removed character.
 */
void String_PopBack (String s);

/*
 * Extends the String by appending char * sequance to the end
 */
void String_Append_cp (String s, const char * c);

/*
 * Extends the String by appending content of another String instance
 */
void String_Append_s (String s, String o);

void String_Append_l (String s, long n);

#define String_Append(a,b)                                           \
    _Generic ((0,b),                                                 \
            char *: String_Append_cp,                                \
            const char *: String_Append_cp,                          \
            struct String_t *: String_Append_s,                      \
            int: String_Append_l,                                    \
            long: String_Append_l                                    \
            )(a,b)

/*
 * Returns a reference to the first character in the string. Essentially the same as String_Data
 * but does additional check for size.
 */
const char * String_Front (const String s);

/*
 * Returns a reference to the last character in the string.
 */
const char * String_Back (const String s);

/*
 * Returns the number of characters in the string.
 * This is the number of actual objects held in the string, which is not necessarily equal to
 * its storage capacity.
 */
size_t String_Size (const String s);

/*
 * Returns the size of the storage space currently allocated for the string, expressed in terms
 * of chars.
 *
 * This capacity is not necessarily equal to the string size. It can be equal or greater, with
 * the extra space allowing to accommodate for growth without the need to reallocate on each
 * insertion.
 *
 * Notice that this capacity does not suppose a limit on the size of the string. When this
 * capacity is exhausted and more is needed, it is automatically expanded by the container
 * (reallocating it storage space).
 *
 * The capacity of a string can be explicitly altered by calling String_Reserve.
 */
size_t String_Capacity (const String s);

/*
 * Returns a reference to the character at position n in the string.
 *
 * The function automatically checks whether n is within the bounds of valid characters in
 * the string.
 */
const char * String_At (const String s, size_t pos);

/*
 * Returns a direct pointer to the memory array used internally by the string to store its owned
 * characters.
 *
 * Because characters in the string are guaranteed to be stored in contiguous storage locations
 * in the same order as represented by the string, the pointer retrieved can be offset to access
 * any character in the array.
 */
void * String_Data (const String s);

#define String_PrintF(s, ...) printf(s->data, __VA_ARGS__);

#endif /* end of include guard: STRING_H_A4HVP9OX */
