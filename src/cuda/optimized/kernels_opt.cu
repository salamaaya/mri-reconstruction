/* src/cuda/optimized/kernels_opt.cu */

#include "complex.h"

#define BLOCK_DIM_X 32
#define BLOCK_DIM_Y 32
#define BLOCK_SIZE (BLOCK_DIM_X * BLOCK_DIM_Y)

#define OPT_STRAT_SHARED   1
#define OPT_STRAT_TEXTURED 2
#define OPT_STRAT          OPT_STRAT_SHARED

/* 
* Forward NUFFT  (image → k-space samples)
*
* Tiling strategy:
*   Each thread owns one sample 's' and must sum over every image pixel.
*   All threads in the 1-D block share the same image tile, so we load
*   BLOCK_SIZE pixels at a time into shared memory and let every thread
*   iterate over the local copy instead of hitting global memory.
**/
__global__ void nufft_forward_kernel(const Complex *img, int nx, int ny,
                                     const float *kx, const float *ky, int m,
                                     Complex *samples)
{
    const int s   = blockIdx.x * blockDim.x + threadIdx.x;
    const int tid = threadIdx.x;

    __shared__ Complex sh_img[BLOCK_SIZE];

    const float two_pi   = 2.0f * (float)M_PI;
    const float norm     = 1.0f / sqrtf((float)(nx * ny));
    const float x_center = 0.5f * (float)(nx - 1);
    const float y_center = 0.5f * (float)(ny - 1);

    const float kx_s = (s < m) ? kx[s] : 0.0f;
    const float ky_s = (s < m) ? ky[s] : 0.0f;

    float sum_r = 0.0f, sum_i = 0.0f;

    const int total_pixels = nx * ny;

    for (int tile_start = 0; tile_start < total_pixels; tile_start += BLOCK_SIZE) {
        const int load_idx = tile_start + tid;
        if (load_idx < total_pixels) {
#if OPT_STRAT == OPT_STRAT_SHARED
          sh_img[tid] = img[load_idx];
#elif OPT_STRAT == OPT_STRAT_TEXTURED
          sh_img[tid] = *reinterpret_cast<const Complex*>(
              &__ldg(reinterpret_cast<const float2*>(&img[load_idx]))
          );
#endif
        } else {
          sh_img[tid] = { 0.0f, 0.0f };
        }
        __syncthreads();

        const int tile_len = min(BLOCK_SIZE, total_pixels - tile_start);

        for (int i = 0; i < tile_len; i++) {
            const int pixel_idx = tile_start + i;
            const float xp = (float)(pixel_idx % nx) - x_center;
            const float yp = (float)(pixel_idx / nx) - y_center;

            const float phase = two_pi * (kx_s * xp + ky_s * yp);
            float sin_val, cos_val;
            __sincosf(phase, &sin_val, &cos_val);
            sin_val = -sin_val;

            const Complex v = sh_img[i];
            sum_r += v.real * cos_val - v.imaginary * sin_val;
            sum_i += v.real * sin_val + v.imaginary * cos_val;
        }

        __syncthreads();
    }

    if (s < m) {
        samples[s].real      = norm * sum_r;
        samples[s].imaginary = norm * sum_i;
    }
}

/*
* Adjoint NUFFT  (k-space samples → image)
*
* Tiling strategy:
*   Each thread owns one pixel (x,y) and must sum over every sample.
*   All threads in the 2-D block read the same samples, so we stage
*   BLOCK_SIZE (samples, kx, ky) triplets into shared memory per round.
**/
__global__ void nufft_adjoint_kernel(const Complex *samples, int m,
                                     const float *kx, const float *ky,
                                     int nx, int ny, Complex *img)
{
    const int x   = blockIdx.x * blockDim.x + threadIdx.x;
    const int y   = blockIdx.y * blockDim.y + threadIdx.y;
    const int tid = threadIdx.y * blockDim.x + threadIdx.x;

    __shared__ Complex sh_samples[BLOCK_SIZE];
    __shared__ float   sh_kx[BLOCK_SIZE];
    __shared__ float   sh_ky[BLOCK_SIZE];

    const float two_pi   = 2.0f * (float)M_PI;
    const float norm     = 1.0f / sqrtf((float)(nx * ny));
    const float x_center = 0.5f * (float)(nx - 1);
    const float y_center = 0.5f * (float)(ny - 1);

    const float xp = (float)x - x_center;
    const float yp = (float)y - y_center;

    float sum_r = 0.0f, sum_i = 0.0f;

    for (int tile_start = 0; tile_start < m; tile_start += BLOCK_SIZE) {
        const int load_idx = tile_start + tid;
        if (load_idx < m) {
#if OPT_STRAT == OPT_STRAT_SHARED
          sh_samples[tid] = samples[load_idx];
          sh_kx[tid] = kx[load_idx];
          sh_ky[tid] = ky[load_idx];
#elif OPT_STRAT == OPT_STRAT_TEXTURED
          sh_samples[tid] = *reinterpret_cast<const Complex*>(
                &__ldg(reinterpret_cast<const float2*>(&samples[load_idx]))
          );
          sh_kx[tid] = __ldg(&kx[load_idx]);
          sh_ky[tid] = __ldg(&ky[load_idx]);
#endif
        } else {
            sh_samples[tid] = {0.0f, 0.0f};
            sh_kx[tid]      = 0.0f;
            sh_ky[tid]      = 0.0f;
        }
        __syncthreads();

        const int tile_len = min(BLOCK_SIZE, m - tile_start);

        for (int i = 0; i < tile_len; i++) {
            const float phase = two_pi * (sh_kx[i] * xp + sh_ky[i] * yp);
            float sin_val, cos_val;
            __sincosf(phase, &sin_val, &cos_val);

            const Complex v = sh_samples[i];
            sum_r += v.real * cos_val - v.imaginary * sin_val;
            sum_i += v.real * sin_val + v.imaginary * cos_val;
        }

        __syncthreads();
    }

    if (x < nx && y < ny) {
        img[y * nx + x].real      = norm * sum_r;
        img[y * nx + x].imaginary = norm * sum_i;
    }
}

