/* include/nufft.h */

#ifndef NUFFT_H
#define NUFFT_H

#include "complex.h"

void nufft_forward(const Complex *img, int nx, int ny,
                   const float *kx, const float *ky, int m,
                   Complex *samples);
void nufft_adjoint(const Complex *samples, int m,
                   const float *kx, const float *ky,
                   int nx, int ny, Complex *img);
void nufft(const Complex *img, int nx, int ny,
                   const float *kx, const float *ky, int m,
                   Complex *samples, Complex *out);

#endif /* NUFFT_H */
