#ifndef NUFFT_H
#define NUFFT_H

#include "complex.h"

/*
 * Forward NUFFT (image -> k-space samples).
 * img has size nx * ny, samples has size m.
 */
void nufft_forward(const Complex *img, int nx, int ny,
                   const float *kx, const float *ky, int m,
                   Complex *samples);

/*
 * Adjoint NUFFT (k-space samples -> image).
 * samples has size m, img has size nx * ny.
 */
void nufft_adjoint(const Complex *samples, int m,
                   const float *kx, const float *ky,
                   int nx, int ny, Complex *img);

#endif /* NUFFT_H */
