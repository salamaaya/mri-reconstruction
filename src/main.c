/* main.c */

#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"
#include "kspace.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: mri_recon [*.h5]\n");
        return EXIT_FAILURE;
    }

    char *h5file = argv[1];
    KSpace *kspace = read_kspace(h5file);
    if (kspace == NULL) {
        fprintf(stderr, "failed to read kspace data.\n");
        return EXIT_FAILURE;
    }

    printf("kspace dimensions: %d slices x %d rows x %d cols\n",
           kspace->slices, kspace->rows, kspace->cols);

    int s = 0, r = 0, c = 25;
    Complex sample = kspace->data[s * (kspace->rows * kspace->cols) + r * kspace->cols + c];
    printf("kspace[%d][%d][%d] = %.6f + %.6fi\n", s, r, c, sample.real, sample.imaginary);

    Matrix *a = matrix_create(3, 3);
    matrix_setat(a, 1, 0, 0); matrix_setat(a, 1, 0, 1); matrix_setat(a, 1, 0, 2);
    matrix_setat(a, 2, 1, 0); matrix_setat(a, 2, 1, 1); matrix_setat(a, 2, 1, 2);
    matrix_setat(a, 3, 2, 0); matrix_setat(a, 3, 2, 1); matrix_setat(a, 3, 2, 2);

    Matrix *b = matrix_create(3, 3);
    matrix_setat(b, 1, 0, 0); matrix_setat(b, 1, 0, 1); matrix_setat(b, 1, 0, 2);
    matrix_setat(b, 2, 1, 0); matrix_setat(b, 2, 1, 1); matrix_setat(b, 2, 1, 2);
    matrix_setat(b, 3, 2, 0); matrix_setat(b, 3, 2, 1); matrix_setat(b, 3, 2, 2);

    Matrix *sum = matrix_add(a, b);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++)
            printf("%lf ", matrix_getat(sum, i, j));
        printf("\n");
    }

    matrix_destroy(a);
    matrix_destroy(b);
    matrix_destroy(sum);

    free(kspace->data);
    free(kspace);

    return EXIT_SUCCESS;
}

