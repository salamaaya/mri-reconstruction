/* include/ksapce.h */

#include "complex.h"

typedef struct {
    int slices;
    int rows;
    int cols;
    Complex *data;
} KSpace;

KSpace* read_kspace(const char *);

