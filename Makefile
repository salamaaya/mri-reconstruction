CC      = gcc
NVCC    = nvcc
CFLAGS  = -O2 -march=native -Wall -Iinclude
NVFLAGS = -O2 -arch=sm_86 -Iinclude

OUT_DIR	= build
SRC_CPU_DIR = src/cpu

CPU_SRCS  = $(SRC_CPU_DIR)/nufft.c $(SRC_CPU_DIR)/solver.c src/main.c \
			$(SRC_CPU_DIR)/complex.c $(SRC_CPU_DIR)/kspace.c $(SRC_CPU_DIR)/matrix.c
CUDA_SRCS = src/cuda/nufft.cu src/cuda/solver.cu src/cuda/kernels.cu
OPT_SRCS  = src/cuda/optimized/nufft_opt.cu src/cuda/optimized/fft_opt.cu

CFLAGS_DBG  = -g3 -O0 -Wall -Wextra -Iinclude -DDEBUG
NVFLAGS_DBG = -g -G -O0 -arch=sm_86 -Iinclude -lineinfo

.PHONY: all cpu cuda cuda_opt clean

all: cpu cuda cuda_opt

cpu:
	$(CC) $(CFLAGS) $(CPU_SRCS) -o $(OUT_DIR)/mri_recon_cpu -lhdf5

cuda:
	$(NVCC) $(NVFLAGS) $(CPU_SRCS) $(CUDA_SRCS) -o $(OUT_DIR)/mri_recon_gpu -lcufft -lhdf5

cuda_opt:
	$(NVCC) $(NVFLAGS) -DOPTIMIZED $(CPU_SRCS) $(CUDA_SRCS) $(OPT_SRCS) \
	    -o $(OUT_DIR)/mri_recon_opt -lcufft -lhdf5

debug_cpu:
	$(CC) $(CFLAGS_DBG) $(CPU_SRCS) -o $(OUT_DIR)/mri_recon_cpu_dbg -lhdf5

debug_cuda:
	$(NVCC) $(NVFLAGS_DBG) $(CPU_SRCS) $(CUDA_SRCS) -o $(OUT_DIR)/mri_recon_gpu_dbg -lcufft \
		-lhdf5

clean:
	rm -f $(OUT_DIR)/mri_recon_cpu $(OUT_DIR)/mri_recon_gpu $(OUT_DIR)/mri_recon_opt \
			$(OUT_DIR)/mri_recon_cpu_dbg $(OUT_DIR)/mri_recon_gpu_dbg
