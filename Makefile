CC      = gcc
NVCC    = nvcc
CFLAGS  = -O2 -march=native -Wall -Iinclude
NVFLAGS = -O2 -arch=sm_86 -Iinclude

CPU_SRCS  = src/cpu/nufft.c src/cpu/solver.c src/cpu/utils.c src/main.c src/cpu/types.c
CUDA_SRCS = src/cuda/nufft.cu src/cuda/solver.cu src/cuda/kernels.cu
OPT_SRCS  = src/cuda/optimized/nufft_opt.cu src/cuda/optimized/fft_opt.cu

CFLAGS_DBG  = -g3 -O0 -Wall -Wextra -Iinclude -DDEBUG
NVFLAGS_DBG = -g -G -O0 -arch=sm_86 -Iinclude -lineinfo

OUT_DIR	= build

.PHONY: all cpu cuda cuda_opt clean

all: cpu cuda cuda_opt

cpu:
	$(CC) $(CFLAGS) $(CPU_SRCS) -o $(OUT_DIR)/mri_recon_cpu -lm -lhdf5

cuda:
	$(NVCC) $(NVFLAGS) $(CPU_SRCS) $(CUDA_SRCS) -o $(OUT_DIR)/mri_recon_gpu -lcufft -lm -lhdf5

cuda_opt:
	$(NVCC) $(NVFLAGS) -DOPTIMIZED $(CPU_SRCS) $(CUDA_SRCS) $(OPT_SRCS) \
	    -o $(OUT_DIR)/mri_recon_opt -lcufft -lm -lhdf5

debug_cpu:
	$(CC) $(CFLAGS_DBG) $(CPU_SRCS) -o $(OUT_DIR)/mri_recon_cpu_dbg -lm -lhdf5

debug_cuda:
	$(NVCC) $(NVFLAGS_DBG) $(CPU_SRCS) $(CUDA_SRCS) -o $(OUT_DIR)/mri_recon_gpu_dbg -lcufft \
		-lm -lhdf5

clean:
	rm -f mri_recon_cpu mri_recon_gpu mri_recon_opt
