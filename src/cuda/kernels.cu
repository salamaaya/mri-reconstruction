/* src/cuda/kernels.cu */

#include "complex.h"

/* Row-major index helper for (x, y) image coordinates. */
static inline int img_index(int y, int x, int nx)
{
    return y * nx + x;
}

/*
 * Forward NUFFT (image -> k-space samples).
 * img has size nx * ny, samples has size m.
 */
__global__ void nufft_forward(const Complex *img, int nx, int ny,
                              const float *kx, const float *ky, int m,
                              Complex *samples)
{
    int s = blockIdx.x * blockDim.x + threadIdx.x;
    if (s >= m) return;

    const float two_pi = 2.0f * (float)M_PI;
    const float norm = 1.0f / sqrtf((float)(nx * ny));
    const float x_center = 0.5f * (float)(nx - 1);
    const float y_center = 0.5f * (float)(ny - 1);

    float sum_r = 0.0f;
    float sum_i = 0.0f;

    for (int y = 0; y < ny; y++) {
        const float yp = (float)y - y_center;
        for (int x = 0; x < nx; x++) {
            const float xp = (float)x - x_center;
            const float phase = two_pi * (kx[s] * xp + ky[s] * yp);
            const float c = cosf(phase);
            const float sgn_sin = -sinf(phase);
            const Complex v = img[img_index(y, x, nx)];

            sum_r += v.real * c - v.imaginary * sgn_sin;
            sum_i += v.real * sgn_sin + v.imaginary * c;
        }
    }

    samples[s].real = norm * sum_r;
    samples[s].imaginary = norm * sum_i;
}

/*
 * Adjoint NUFFT (k-space samples -> image).
 * samples has size m, img has size nx * ny.
 */
__global__ void nufft_adjoint_kernel(const Complex* samples, int m,
                                     const float* kx, const float* ky,
                                     int nx, int ny, Complex* img)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= nx || y >= ny) return;

    const float two_pi   = 2.0f * CUDART_PI_F;
    const float norm     = 1.0f / sqrtf((float)(nx * ny));
    const float x_center = 0.5f * (float)(nx - 1);
    const float y_center = 0.5f * (float)(ny - 1);

    const float xp = (float)x - x_center;
    const float yp = (float)y - y_center;

    float sum_r = 0.0f;
    float sum_i = 0.0f;

    for (int s = 0; s < m; s++) {
        const float phase = two_pi * (kx[s] * xp + ky[s] * yp);
        const float c = cosf(phase);
        const float sgn_sin = sinf(phase);
        const Complex v = samples[s];
        sum_r += v.real * c - v.imaginary * sgn_sin;
        sum_i += v.real * sgn_sin + v.imaginary * c;
    }

    img[y * nx + x].real      = norm * sum_r;
    img[y * nx + x].imaginary = norm * sum_i;
}

