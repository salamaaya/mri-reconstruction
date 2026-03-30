/* main.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "complex.h"
#include "kspace.h"
#include "solver.h"

static int write_magnitude_volume(const char *path,
                                  const float *data,
                                  int slices,
                                  int rows,
                                  int cols)
{
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open output file: %s\n", path);
        return -1;
    }

    if (fprintf(fp, "MRI_RECON_MAG_V1\n%d %d %d\n", slices, rows, cols) < 0) {
        fclose(fp);
        return -1;
    }

    size_t count = (size_t)slices * (size_t)rows * (size_t)cols;
    if (fwrite(data, sizeof(float), count, fp) != count) {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

static void build_cartesian_trajectory(int rows, int cols, float *kx, float *ky)
{
    int idx = 0;
    const float cx = 0.5f * (float)cols;
    const float cy = 0.5f * (float)rows;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            kx[idx] = ((float)c - cx) / (float)cols;
            ky[idx] = ((float)r - cy) / (float)rows;
            idx++;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: mri_recon [input.h5] [output.mag]\n");
        return EXIT_FAILURE;
    }

    char *h5file = argv[1];
    const char *output = (argc == 3) ? argv[2] : "recon_magnitude.mag";

    KSpace *kspace = kspace_construct(h5file);

    printf("kspace dimensions: %d slices x %d rows x %d cols\n",
           kspace->slices, kspace->rows, kspace->cols);

    const int slices = kspace->slices;
    const int rows = kspace->rows;
    const int cols = kspace->cols;
    const int m = rows * cols;
    const int n = rows * cols;

    float *kx = (float *)malloc((size_t)m * sizeof(float));
    float *ky = (float *)malloc((size_t)m * sizeof(float));
    Complex *rho = (Complex *)malloc((size_t)n * sizeof(Complex));
    float *mag_volume = (float *)malloc((size_t)slices * (size_t)n * sizeof(float));

    if (!kx || !ky || !rho || !mag_volume) {
        fprintf(stderr, "malloc failed (out of memory?).\n");
        free(kx);
        free(ky);
        free(rho);
        free(mag_volume);
        kspace_destroy(kspace);
        return EXIT_FAILURE;
    }

    build_cartesian_trajectory(rows, cols, kx, ky);

    const float lambda = 1e-3f;
    const int max_iter = 30;
    const float tol = 1e-5f;

    for (int s = 0; s < slices; s++) {
        const Complex *samples = &kspace->data[s * m];
        const int iters = cg_reconstruct_slice(samples, kx, ky, m,
                                               cols, rows,
                                               lambda, max_iter, tol,
                                               1, rho);
        if (iters < 0) {
            fprintf(stderr, "Reconstruction failed for slice %d.\n", s);
            free(kx);
            free(ky);
            free(rho);
            free(mag_volume);
            kspace_destroy(kspace);
            return EXIT_FAILURE;
        }

        for (int i = 0; i < n; i++) {
            const float rr = rho[i].real;
            const float ri = rho[i].imaginary;
            mag_volume[s * n + i] = sqrtf(rr * rr + ri * ri);
        }

        printf("slice %d/%d reconstructed in %d CG iterations\n", s + 1, slices, iters);
    }

    if (write_magnitude_volume(output, mag_volume, slices, rows, cols) != 0) {
        fprintf(stderr, "Failed to write output volume: %s\n", output);
        free(kx);
        free(ky);
        free(rho);
        free(mag_volume);
        kspace_destroy(kspace);
        return EXIT_FAILURE;
    }

    printf("wrote reconstructed magnitude volume: %s\n", output);

    free(kx);
    free(ky);
    free(rho);
    free(mag_volume);

    kspace_destroy(kspace);

    return EXIT_SUCCESS;
}
