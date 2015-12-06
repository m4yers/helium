#ifndef PAIR_H_E8LKNU5A
#define PAIR_H_E8LKNU5A

#include <stddef.h>

typedef struct Pair_t * Pair;

struct Pair_t
{
    const void * first;
    const void * second;
};

void Pair_Init_ (Pair p, size_t fs, const void * fd, size_t ss, const void * sd);

#define Pair_Init(p, f, s) \
    {\
        __typeof__((0, f)) __f = f;\
        __typeof__((0, s)) __s = s;\
        Pair_Init_ (p, sizeof(__f), &__f, sizeof(__s), &__s);\
    }

void Pair_Fini (Pair p);

Pair Pair_New_ (size_t fs, const void * fd, size_t ss, const void * sd);

#define Pair_New(f,s) Pair_New_(sizeof(f), &f, sizeof(s), &s)

#define Pair_Delete(p) Pair_Fini(p); free(p); p = NULL;

/*
 * Sets the Pair instance onto a new memory block with corresponind data types
 */
#define Pair_SetOn(p,m,f,s) \
    {\
        p->first = m;\
        p->second = m + sizeof(f);\
    }

const void * Pair_First (Pair p);
const void * Pair_Second (Pair p);

#endif /* end of include guard: PAIR_H_E8LKNU5A */
