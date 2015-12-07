#ifndef VECTOR_H_O3R2FVGL
#define VECTOR_H_O3R2FVGL

#include <stddef.h>

#include "bool.h"

typedef void (*VectorElementDestructor) (void * value);

typedef struct Vector_t * Vector;

struct Vector_t
{
    char * data;

    /*
     * allocated = type_size * capacity
     */
    size_t allocated;

    /*
     * size of items in vector
     */
    size_t type_size;

    /*
     * The size of the storage space currently allocated for the vector, expressed in terms of
     * elements.
     */
    size_t capacity;

    /*
     * This is the number of actual objects held in the vector, which is not necessarily equal
     * to its storage capacity.
     */
    size_t size;

    /*
     * Function used to destroy a vector element.
     */
    VectorElementDestructor dest;
};

#define Vector(t) {\
    .data = checked_malloc(sizeof(t) * 4),\
    .allocated = sizeof(t) * 4,\
    .type_size = sizeof(t),\
    .capacity = 4,\
    .size = 0,\
    .dest = NULL}

/*
 * Initializes existing or creates a new vector.
 *
 * For convenience you could use Vector_New macro to be able create Vector by providing type
 * directly without specifying its size
 */
Vector _Vector_Init (Vector v, size_t type_size, size_t n);

#define Vector_Init(vector, type) _Vector_Init (vector, sizeof (type), 4);

/*
 * Frees all memory allocated for the vector.
 */
void Vector_Fini (Vector v);

/*
 * Same as Vector_Init but the instance is allocated on heap
 */
Vector _Vector_New (size_t type_size, size_t n);

#define Vector_New(type) _Vector_New (sizeof (type), 4);
/*
 * Same as Vector_Fini plus the instance is freed and assigned to NULL.
 */
void Vector_Delete (Vector v);

/*
 * Destructor used to destroy an element if it is about to be removed from vector, e.g. Vector_Clear.
 */
void Vector_SetDestructor (Vector v, VectorElementDestructor f);

/*
 * Returns whether the vector is empty (i.e. whether its size is 0). This function does not
 * modify the container in any way. To clear the content of a vector, see Vector_Clear.
 */
bool Vector_Empty (const Vector v);

/*
 * Removes all elements from the vector (which are destroyed), leaving the container with a
 * size of 0.
 */
void Vector_Clear (Vector v);

/*
 * Requests that the vector capacity be at least enough to contain n elements.
 *
 *
 * If n is greater than the current vector capacity, the function causes the container to
 * reallocate its storage increasing its capacity to n (or greater).
 *
 * In all other cases, the function call does not cause a reallocation and the vector capacity
 * is not affected.
 *
 * This function has no effect on the vector size and cannot alter its elements.
 */
void Vector_Reserve (Vector v, size_t n);

/*
 * Resizes the container so that it contains n elements.
 *
 * If n is smaller than the current container size, the content is reduced to its first n
 * elements. The actual data won't be erased unless overwritten by subsequent calls to vector
 * functions. Be cautions though, by truncating the vector using this method you will lose access
 * to your data, this might lead to memory leaks.
 *
 * If n is greater than the current container size, the content is expanded by inserting at
 * the end as many elements as needed to reach a size of n. If val is specified, the new elements
 * are initialized as copies of val, otherwise, they are value-initialized.
 *
 * If n is also greater than the current container capacity, an automatic reallocation of the
 * allocated storage space takes place.
 *
 * Notice that this function changes the actual content of the container by inserting or erasing
 * elements from it.
 */
void Vector_Resize (Vector v, size_t n, const void * value);

/*
 * Adds a new element at the end of the vector, after its current last element. The content of
 * val is copied to the new element.
 *
 * This effectively increases the container size by one, which causes an automatic reallocation
 * of the allocated storage space if -and only if- the new vector size surpasses the current
 * vector capacity.
 *
 * For convenience you could use Vector_PushBack macro to be able push literals and not to use
 * memory addresses directly.
 */
void Vector_Push (Vector v, const void * value);

/*
 * Removes the last element in the vector, effectively reducing the container size by one.
 *
 * This destroys the removed element.
 */
void Vector_PopBack (Vector v);

/*
 * Returns a reference to the first element in the vector. Essentially the same as Vector_Data
 * but does additional check for size.
 */
void * Vector_Front (const Vector v);

/*
 * Returns a reference to the last element in the vector.
 */
void * Vector_Back (const Vector v);

/*
 * Returns the number of elements in the vector.
 * This is the number of actual objects held in the vector, which is not necessarily equal to
 * its storage capacity.
 */
size_t Vector_Size (const Vector v);

/*
 * Returns the size of the storage space currently allocated for the vector, expressed in terms
 * of elements.
 *
 * This capacity is not necessarily equal to the vector size. It can be equal or greater, with
 * the extra space allowing to accommodate for growth without the need to reallocate on each
 * insertion.
 *
 * Notice that this capacity does not suppose a limit on the size of the vector. When this
 * capacity is exhausted and more is needed, it is automatically expanded by the container
 * (reallocating it storage space).
 *
 * The capacity of a vector can be explicitly altered by calling Vector_Reserve.
 */
size_t Vector_Capacity (const Vector v);

/*
 * Returns a reference to the element at position n in the vector.
 *
 * The function automatically checks whether n is within the bounds of valid elements in
 * the vector.
 */
void * Vector_At (const Vector v, size_t pos);

/*
 * Returns a direct pointer to the memory array used internally by the vector to store its owned
 * elements.
 *
 * Because elements in the vector are guaranteed to be stored in contiguous storage locations
 * in the same order as represented by the vector, the pointer retrieved can be offset to access
 * any element in the array.
 */
void * Vector_Data (const Vector v);

/*
 * Ternary operator used to break string literal(array) to char pointer
 */
#define Vector_PushBack(vector, value) \
    {\
        __typeof__(TRUE ? value : value) __value = value;\
        Vector_Push (vector, &__value);\
    }\

#define VECTOR_FOREACH(type, item, vector) \
    for (size_t __index = 0, __size = Vector_Size(vector); __index < __size; ++__index)\
    for (type * item = Vector_At(vector, __index); item; item = NULL)\

#endif /* end of include guard: VECTOR_H_O3R2FVGL */
