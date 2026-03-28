# GPU-Accelerated MRI Reconstruction via Linear Solvers

Accelerating MRI image reconstruction using CUDA by parallelizing the linear solver pipeline. The system implements a simplified NUFFT approximation and iterative solver, comparing a sequential CPU baseline against naive and optimized GPU implementations.

---

## Background

MRI reconstruction transforms raw k-space signal data into interpretable images. This project solves the regularized least-squares system:

```
(F^H F + λ W^H W) ρ = F^H D
```

where:
- `F` — NUFFT matrix modeling the MRI acquisition physics
- `D` — vector of raw k-space samples from the scanner
- `W` — masking/regularization matrix (identity for baseline)
- `ρ` — output voxel values of the reconstructed image
- `λ` — regularization strength

The iterative solver (Conjugate Gradient) is applied to find `ρ`. The bulk of computation — matrix-vector products, NUFFT evaluations, and dot products — is parallelized on the GPU.

---

## Project Structure

```
.
├── src/
│   ├── cpu/
│   │   ├── nufft.c          # CPU NUFFT approximation
│   │   ├── solver.c         # CPU conjugate gradient solver
│   │   ├── types.c          # Types needed for computation such as complex numbers, etc.
│   │   └── utils.c          # k-space I/O, image utilities
│   ├── cuda/
│   │   ├── nufft.cu         # CUDA NUFFT kernels
│   │   ├── solver.cu        # CUDA CG solver
│   │   ├── kernels.cu       # Linear algebra kernels (matvec, dot, axpy)
│   │   └── optimized/
│   │       ├── nufft_opt.cu # Shared memory / tiled NUFFT
│   │       └── fft_opt.cu   # cuFFT-based optimizations
│   └── main.c               # Entry point
├── include/
│   ├── nufft.h
│   ├── solver.h
│   ├── types.h 
│   └── utils.h
├── data/                    # k-space input data (not tracked)
├── results/                 # Reconstruction outputs and benchmarks
├── tests/
│   ├── test_nufft.c
│   └── test_solver.c
├── Makefile
└── README.md
```

---

## Dependencies

| Dependency | Purpose |
|---|---|
| CUDA Toolkit ≥ 11.0 | GPU kernels and cuFFT |
| GCC ≥ 9.0 | CPU baseline compilation |
| cuFFT | Optimized FFT on GPU |
| [fastMRI](https://fastmri.med.nyu.edu/) | MRI k-space datasets |

---

## Build

```bash
# CPU baseline only
make cpu

# Naive CUDA build
make cuda

# Optimized CUDA build
make cuda_opt

# All targets
make all
```

---

## Usage

```bash
# Run CPU baseline reconstruction
./mri_recon --mode cpu --input data/kspace.bin --output results/recon_cpu.bin

# Run naive GPU reconstruction
./mri_recon --mode gpu --input data/kspace.bin --output results/recon_gpu.bin

# Run optimized GPU reconstruction
./mri_recon --mode gpu_opt --input data/kspace.bin --output results/recon_opt.bin

# Run benchmark (all three modes, prints speedup table)
./mri_recon --benchmark --input data/kspace.bin
```

---

## Milestones

| Date | |
|---|---|
| April 1 | Sequential CPU implementation (NUFFT + CG solver) |
| April 15 | Naive CUDA implementation with basic kernel parallelism |
| April 29 | Optimized CUDA (shared memory, memory coalescing, cuFFT) |

---

## Evaluation

Three-way comparison across:
- **Speedup** over CPU baseline
- **Reconstruction time** (wall clock, per iteration)
- **Image quality** (PSNR / SSIM vs. reference)
- **Scalability** across image sizes and k-space sampling rates

Baseline theoretical speedup estimate (Amdahl's Law, 95% parallel fraction, N=1000 workers): **~19.6×**

---

## References

1. Kirk, D. B., & Hwu, W. W. (2022). *Programming Massively Parallel Processors* (4th ed.). Morgan Kaufmann.
2. [NVIDIA GPU Gems 2 – Chapter 48: Medical Image Reconstruction](https://developer.nvidia.com/gpugems/gpugems2/part-vi-simulation-and-numerical-algorithms/chapter-48-medical-image-reconstruction)
3. [fastMRI Dataset – NYU](https://fastmri.med.nyu.edu/)
4. Ramzi et al. (2022). NC-PDNet: A Density-Compensated Unrolled Network for 2D and 3D Non-Cartesian MRI Reconstruction. *IEEE Transactions on Medical Imaging*, 41(7), 1625–1638.
