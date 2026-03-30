/* include/ksapce.h */

#ifndef KSPACE_H
#define KSPACE_H

#include "complex.h"
#include "hdf5.h"

typedef struct {
    int slices;
    int rows;
    int cols;
    hid_t file_id, dataset_id, space_id;
    hid_t complex_t;
    hsize_t *dims, ndims;
    Complex *data;
} KSpace;

KSpace* kspace_construct(const char *);
void    kspace_destroy(KSpace *);

#endif /* KSPACE_H */

