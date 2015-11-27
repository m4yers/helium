#ifndef STRING_H_A4HVP9OX
#define STRING_H_A4HVP9OX

#include <stdlib.h>
#include <stddef.h>

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

/*
 * Compares two string including nul charactesrs and returns -1 if two String instances contain
 * equal sequence of characters. If the two String instances are different the index of the first
 * unmatched character is returned.
 */
int String_Diff (String a, String b);

/*
 * Returns True if Diff of two Strings returns -1.
 */
bool String_Equal (String a, String b);

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
 * Returns a reference to the first character in the string. Essentially the same as Vector_Data
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
 * The capacity of a string can be explicitly altered by calling Vector_Reserve.
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
