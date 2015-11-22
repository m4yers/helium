#ifndef UTIL_H_T8MIWUHH
#define UTIL_H_T8MIWUHH

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef char * string;

string String (char *);

char * itoa (int value, char * result, int base);

#endif /* end of include guard: UTIL_H_T8MIWUHH */
