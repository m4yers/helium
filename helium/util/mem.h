#ifndef MEM_H_GE4BLQDT
#define MEM_H_GE4BLQDT

#include <stddef.h>

void * aligned_malloc (size_t required_bytes, size_t alignment);
void aligned_free (void * p);
void * checked_malloc (int);
void * checked_dup (const char * s, size_t n);

#endif /* end of include guard: MEM_H_GE4BLQDT */
