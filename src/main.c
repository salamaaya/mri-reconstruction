/* main.c */

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

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

    free(kspace->data);
    free(kspace);

    return EXIT_SUCCESS;
}
