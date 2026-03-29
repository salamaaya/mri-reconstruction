/* src/cpu/kspace.c */

#include <stdlib.h>

#include "kspace.h"

KSpace* kspace_construct(const char *h5file)
{
    herr_t status;

    KSpace *kspace;
    size_t bytes;

    kspace = (KSpace *)malloc(sizeof(KSpace));
    if (!kspace) {
        fprintf(stderr, "malloc failed (out of memory?).\n");
        exit(EXIT_FAILURE);
    }

    kspace->file_id = H5Fopen(h5file, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (kspace->file_id < 0) {
        fprintf(stderr, "Failed to open %s.\n", h5file);
        exit(EXIT_FAILURE);
    }

    kspace->dataset_id = H5Dopen2(kspace->file_id, "/kspace", H5P_DEFAULT);
    if (kspace->dataset_id < 0) {
        fprintf(stderr, "Failed to open dataset from %s.\n", h5file);
        exit(EXIT_FAILURE);
    }

    kspace->complex_t = H5Tcreate(H5T_COMPOUND, sizeof(Complex));
    H5Tinsert(kspace->complex_t, "r", HOFFSET(Complex, real), H5T_NATIVE_FLOAT);
    H5Tinsert(kspace->complex_t, "i", HOFFSET(Complex, imaginary), H5T_NATIVE_FLOAT);

    kspace->space_id = H5Dget_space(kspace->dataset_id);
    kspace->ndims = H5Sget_simple_extent_ndims(kspace->space_id);
    kspace->dims = (hsize_t *)malloc(kspace->ndims * sizeof(hsize_t));
    H5Sget_simple_extent_dims(kspace->space_id, kspace->dims, NULL);

    kspace->slices = (int)kspace->dims[0];
    kspace->rows = (int)kspace->dims[1];
    kspace->cols = (int)kspace->dims[2];

    bytes = kspace->slices * kspace->rows * kspace->cols * sizeof(Complex);
    kspace->data = (Complex *)malloc(bytes);
    if (!kspace->data) {
        fprintf(stderr, "malloc failed (out of memory?).\n");
        kspace_destroy(kspace);
        exit(EXIT_FAILURE);
    }

    status = H5Dread(kspace->dataset_id, kspace->complex_t, H5S_ALL, H5S_ALL, H5P_DEFAULT, kspace->data);
    if (status < 0) {
        fprintf(stderr, "Failed to read dataset.\n");
        kspace_destroy(kspace);
        exit(EXIT_FAILURE);
    }

    return kspace;
}

void kspace_destroy(KSpace *kspace)
{
    if (kspace->data) free(kspace->data);
    if (kspace->complex_t) H5Tclose(kspace->complex_t);
    if (kspace->dataset_id) H5Dclose(kspace->dataset_id);
    if (kspace->file_id) H5Fclose(kspace->file_id);
    if (kspace->dims) free(kspace->dims);
    free(kspace);
}

