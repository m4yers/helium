#ifndef UTIL_H_T8MIWUHH
#define UTIL_H_T8MIWUHH

#include "util/mem.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

char * itoa (int value, char * result, int base);

#define U_Create(T,n)                                            \
    T n = checked_malloc (sizeof (*n));                          \
    *n = (__typeof__ (*n))                                       \

#define U_CreateDefault(T,n)                                     \
    T n = checked_malloc (sizeof (*n));                          \
    *n = (__typeof__ (*n)){0};                                   \

#endif /* end of include guard: UTIL_H_T8MIWUHH */
