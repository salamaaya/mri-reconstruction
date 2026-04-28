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
│   │   ├── complex.c
│   │   ├── kspace.c
│   │   ├── vector.c
│   │   ├── nufft.c
│   │   └── solver.c 
│   ├── cuda/
│   │   ├── nufft.cu
│   │   ├── solver.cu
│   │   ├── kernels.cu
│   │   └── optimized/
│   │       ├── nufft_opt.cu
│   │       └── kernels_opt.cu
│   └── main.c
├── include/
│   ├── complex.h
│   ├── kspace.h
│   ├── vector.h
│   ├── nufft.h
│   └── solver.h
├── data/
├── results/
├── experiment/
│   └── experiment.py
├── Makefile
└── README.md
```

---

## Dependencies

| Dependency | Purpose | Example Installation |
|---|---| --- |
| CUDA Toolkit ≥ 11.0 | GPU kernels ||
| GCC ≥ 9.0 | CPU baseline compilation | `sudo dnf install gcc` |
| libhdf5 | Reading fastMRI `.h5` files | `sudo dnf install hdf5-devel.x86_64` |
| [fastMRI](https://fastmri.med.nyu.edu/) | MRI k-space datasets | by request |

---

## Build

```bash
mkdir build

# CPU baseline only
make cpu

# Naive CUDA build
make cuda

# Optimized CUDA build
make cuda_opt

# Optimized CUDA build with shared memory
make cuda_opt_shared

# Optimized CUDA build with textured memory
make cuda_opt_textured

# All targets
make all
```

---

## Usage

```bash
# Run CPU baseline reconstruction
./build/mri_recon_cpu [data/*.h5] [results/*]

# Run naive GPU reconstruction
./build/mri_recon_cuda [data/*.h5] [results/*]

# Run optimized GPU reconstruction with shared memory
./build/mri_recon_cuda_opt_shared [data/*.h5] [results/*]

# Run optimized GPU reconstruction with textured memory
./build/mri_recon_cuda_opt_textured [data/*.h5] [results/*]
```

---

## Milestones

| Date | |
|---|---|
| April 1 | Sequential CPU implementation (NUFFT + CG solver) |
| April 15 | Naive CUDA implementation with basic kernel parallelism |
| April 29 | Optimized CUDA (shared memory, memory coalescing, etc.) |

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
5. https://akrause2014.github.io/HDF5-Basics/c/html/#viewing-the-contents-of-an-hdf5-file
