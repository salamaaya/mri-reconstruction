/* src/cuda/optimized/nufft_opt.cu */

#include "complex.h"
#include "nufft.h"

#define BLOCK_DIM_X 32
#define BLOCK_DIM_Y 32
#define BLOCK_SIZE (BLOCK_DIM_X * BLOCK_DIM_Y)

__global__ void nufft_forward_kernel(const Complex *, int, int,
                   const float *, const float *, int, Complex *);
__global__ void nufft_adjoint_kernel(const Complex *, int,
                   const float *, const float *,
                   int, int, Complex *);

void nufft_adjoint(const Complex *samples, int m,
                   const float *kx, const float *ky,
                   int nx, int ny,
                   Complex *out)
{
    Complex *samples_d, *out_d;
    float *kx_d, *ky_d;

    int bytes_img = nx * ny * sizeof(Complex);
    int bytes_samples = m * sizeof(Complex);
    int bytes_k = m * sizeof(float);

    cudaMalloc(&samples_d, bytes_samples);
    cudaMalloc(&out_d, bytes_img);
    cudaMalloc(&kx_d, bytes_k);
    cudaMalloc(&ky_d, bytes_k);

    cudaMemcpy(samples_d, samples, bytes_samples, cudaMemcpyHostToDevice);
    cudaMemcpy(kx_d, kx, bytes_k, cudaMemcpyHostToDevice);
    cudaMemcpy(ky_d, ky, bytes_k, cudaMemcpyHostToDevice);
    cudaMemset(out_d, 0, bytes_img);

    dim3 blockDim(BLOCK_DIM_X, BLOCK_DIM_Y);
    dim3 gridDim((nx + BLOCK_DIM_X - 1) / BLOCK_DIM_X,
                 (ny + BLOCK_DIM_Y - 1) / BLOCK_DIM_Y);

    nufft_adjoint_kernel<<<gridDim, blockDim>>>(samples_d, m, kx_d, ky_d, nx, ny, out_d);
    cudaDeviceSynchronize();

    cudaMemcpy(out, out_d, bytes_img, cudaMemcpyDeviceToHost);

    cudaFree(samples_d);
    cudaFree(out_d);
    cudaFree(kx_d);
    cudaFree(ky_d);
}

void nufft(const Complex *img, int nx, int ny,
           const float *kx, const float *ky, int m,
           Complex *samples, Complex *out)
{
    Complex *img_d, *samples_d, *out_d;
    float *kx_d, *ky_d;

    int bytes_img = nx * ny * sizeof(Complex);
    int bytes_samples = m * sizeof(Complex);
    int bytes_k = m * sizeof(float);

    cudaMalloc(&img_d, bytes_img);
    cudaMalloc(&samples_d, bytes_samples);
    cudaMalloc(&out_d, bytes_img);
    cudaMalloc(&kx_d, bytes_k);
    cudaMalloc(&ky_d, bytes_k);

    cudaMemcpy(img_d, img, bytes_img, cudaMemcpyHostToDevice);
    cudaMemcpy(samples_d, samples, bytes_samples, cudaMemcpyHostToDevice);
    cudaMemcpy(kx_d, kx, bytes_k, cudaMemcpyHostToDevice);
    cudaMemcpy(ky_d, ky, bytes_k, cudaMemcpyHostToDevice);

    int gridSize = (m + BLOCK_SIZE - 1) / BLOCK_SIZE;
    dim3 blockDim(BLOCK_DIM_X, BLOCK_DIM_Y);
    dim3 gridDim((nx + BLOCK_DIM_X - 1) / BLOCK_DIM_X,
                 (ny + BLOCK_DIM_Y - 1) / BLOCK_DIM_Y);

    nufft_forward_kernel<<<gridSize, BLOCK_SIZE>>>(img_d, nx, ny, kx_d, ky_d, m, samples_d);
    cudaDeviceSynchronize();
    nufft_adjoint_kernel<<<gridDim, blockDim>>>(samples_d, m, kx_d, ky_d, nx, ny, out_d);
    cudaDeviceSynchronize();

    cudaMemcpy(out, out_d, bytes_img, cudaMemcpyDeviceToHost);

    cudaFree(img_d);
    cudaFree(samples_d);
    cudaFree(out_d);
    cudaFree(kx_d);
    cudaFree(ky_d);
}

