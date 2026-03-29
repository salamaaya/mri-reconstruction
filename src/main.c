/* main.c */

#include <stdio.h>
#include <stdlib.h>

#include "complex.h"
#include "matrix.h"
#include "kspace.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: mri_recon [*.h5]\n");
        return EXIT_FAILURE;
    }

    char *h5file = argv[1];
    KSpace *kspace = kspace_construct(h5file);

    printf("kspace dimensions: %d slices x %d rows x %d cols\n",
           kspace->slices, kspace->rows, kspace->cols);

    int s = 0, r = 0, c = 25;
    Complex sample1 = kspace->data[s * (kspace->rows * kspace->cols) + r * kspace->cols + c];
    Complex sample2 = kspace->data[s * (kspace->rows * kspace->cols) + r * kspace->cols + (c + 1)];
    Complex sample3 = kspace->data[s * (kspace->rows * kspace->cols) + r * kspace->cols + (c + 2)];

    Matrix *a = matrix_construct(3, 3);
    matrix_setat(a, sample1, 0, 0); matrix_setat(a, sample1, 0, 1); matrix_setat(a, sample1, 0, 2);
    matrix_setat(a, sample2, 1, 0); matrix_setat(a, sample2, 1, 1); matrix_setat(a, sample2, 1, 2);
    matrix_setat(a, sample3, 2, 0); matrix_setat(a, sample3, 2, 1); matrix_setat(a, sample3, 2, 2);
    printf("a:\n");
    matrix_print(a);

    Matrix *x = matrix_construct(3, 1);
    matrix_setat(x, sample1, 0, 0);
    matrix_setat(x, sample2, 1, 0);
    matrix_setat(x, sample3, 2, 0);
    printf("x:\n");
    matrix_print(x);

    Matrix *sum = matrix_add(a, a);
    printf("a + a:\n");
    matrix_print(sum);

    Matrix *prod = matrix_mul(sum, x);
    printf("sum * x:\n");
    matrix_print(prod);

    Matrix *transpose = matrix_transpose(a);
    printf("a^T:\n");
    matrix_print(transpose);

    Matrix *hermitian = matrix_hermitian(a);
    printf("a^H:\n");
    matrix_print(hermitian);

    matrix_destroy(prod);
    matrix_destroy(a);
    matrix_destroy(sum);
    matrix_destroy(x);
    matrix_destroy(transpose);
    matrix_destroy(hermitian);

    kspace_destroy(kspace);

    return EXIT_SUCCESS;
}

