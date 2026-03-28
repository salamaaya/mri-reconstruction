/* src/cpu/kspace.c */

#include <stdlib.h>

#include "kspace.h"
#include "hdf5.h"

KSpace* read_kspace(const char *h5file)
{
    herr_t status;
    hid_t file_id, dataset_id, space_id;
    hid_t complex_t;
    hsize_t *dims, ndims;

    KSpace *kspace;
    size_t bytes;

    file_id = H5Fopen(h5file, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_id < 0) {
        fprintf(stderr, "Failed to open %s.\n", h5file);
        return NULL;
    }

    dataset_id = H5Dopen2(file_id, "/kspace", H5P_DEFAULT);
    if (dataset_id < 0) {
        fprintf(stderr, "Failed to open dataset from %s.\n", h5file);
        H5Fclose(file_id);
        return NULL;
    }

    complex_t = H5Tcreate(H5T_COMPOUND, sizeof(Complex));
    H5Tinsert(complex_t, "r", HOFFSET(Complex, real), H5T_NATIVE_FLOAT);
    H5Tinsert(complex_t, "i", HOFFSET(Complex, imaginary), H5T_NATIVE_FLOAT);

    space_id = H5Dget_space(dataset_id);
    ndims = H5Sget_simple_extent_ndims(space_id);
    dims = (hsize_t *)malloc(ndims * sizeof(hsize_t));
    H5Sget_simple_extent_dims(space_id, dims, NULL);

    kspace = (KSpace *)malloc(sizeof(KSpace));
    if (!kspace) {
        fprintf(stderr, "malloc failed (out of memory?).\n");
        return NULL;
    }

    kspace->slices = (int)dims[0];
    kspace->rows = (int)dims[1];
    kspace->cols = (int)dims[2];

    bytes = kspace->slices * kspace->rows * kspace->cols * sizeof(Complex);
    kspace->data = (Complex *)malloc(bytes);
    if (!kspace->data) {
        fprintf(stderr, "malloc failed (out of memory?).\n");
        kspace = NULL;
        goto exit;
    }

    status = H5Dread(dataset_id, complex_t, H5S_ALL, H5S_ALL, H5P_DEFAULT, kspace->data);
    if (status < 0) {
        fprintf(stderr, "Failed to read dataset.\n");
        kspace = NULL;
        goto exit;
    }

exit:
    free(dims);

    H5Tclose(complex_t);
    H5Dclose(dataset_id);
    H5Fclose(file_id);

    return kspace;
}
