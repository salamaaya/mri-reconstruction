/* main.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "complex.h"
#include "kspace.h"
#include "solver.h"

static int write_pgm_slice(const char *path,
                           const float *data,
                           int rows,
                           int cols)
{
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open output file: %s\n", path);
        return -1;
    }

    if (fprintf(fp, "P5\n%d %d\n255\n", cols, rows) < 0) {
        fclose(fp);
        return -1;
    }

    const int n = rows * cols;
    float min_v = data[0];
    float max_v = data[0];

    for (int i = 1; i < n; i++) {
        if (data[i] < min_v) min_v = data[i];
        if (data[i] > max_v) max_v = data[i];
    }

    const float range = max_v - min_v;
    unsigned char *pixels = (unsigned char *)malloc((size_t)n);
    if (!pixels) {
        fclose(fp);
        return -1;
    }

    for (int i = 0; i < n; i++) {
        float norm = (range > 1e-12f) ? (data[i] - min_v) / range : 0.0f;
        int val = (int)(norm * 255.0f + 0.5f);
        if (val < 0) val = 0;
        if (val > 255) val = 255;
        pixels[i] = (unsigned char)val;
    }

    if (fwrite(pixels, 1, (size_t)n, fp) != (size_t)n) {
        free(pixels);
        fclose(fp);
        return -1;
    }

    free(pixels);
    fclose(fp);
    return 0;
}

static int write_pgm_volume(const char *prefix,
                            const float *data,
                            int slices,
                            int rows,
                            int cols)
{
    char path[512];
    const int n = rows * cols;

    for (int s = 0; s < slices; s++) {
        if (slices == 1) {
            if (snprintf(path, sizeof(path), "%s.pgm", prefix) >= (int)sizeof(path))
                return -1;
        } else {
            if (snprintf(path, sizeof(path), "%s_slice_%03d.pgm", prefix, s) >= (int)sizeof(path))
                return -1;
        }

        if (write_pgm_slice(path, &data[s * n], rows, cols) != 0)
            return -1;
    }

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
        fprintf(stderr, "usage: mri_recon [input.h5] [output_prefix]\n");
        return EXIT_FAILURE;
    }

    char *h5file = argv[1];
    const char *output_prefix = (argc == 3) ? argv[2] : "recon_magnitude";

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

    if (write_pgm_volume(output_prefix, mag_volume, slices, rows, cols) != 0) {
        fprintf(stderr, "Failed to write PGM output with prefix: %s\n", output_prefix);
        free(kx);
        free(ky);
        free(rho);
        free(mag_volume);
        kspace_destroy(kspace);
        return EXIT_FAILURE;
    }

    if (slices == 1)
        printf("wrote reconstructed image: %s.pgm\n", output_prefix);
    else
        printf("wrote %d reconstructed slices: %s_slice_###.pgm\n", slices, output_prefix);

    free(kx);
    free(ky);
    free(rho);
    free(mag_volume);

    kspace_destroy(kspace);

    return EXIT_SUCCESS;
}
