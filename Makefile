CC      = gcc
NVCC    = nvcc
CFLAGS  = -O2 -march=native -Wall -Iinclude
NVFLAGS = -O2 -Iinclude -arch=sm_75

HDF5_CFLAGS := $(shell pkg-config --cflags hdf5 2>/dev/null)
HDF5_LIBS   := $(shell pkg-config --libs hdf5 2>/dev/null)
HDF5_LINK   := $(if $(HDF5_LIBS),$(HDF5_LIBS),-lhdf5)

CFLAGS  += $(HDF5_CFLAGS)
NVFLAGS += $(HDF5_CFLAGS)

OUT_DIR     = build
SRC_CPU_DIR = src/cpu

# Shared C files that are always included (no GPU equivalent)
COMMON_SRCS = src/main.c \
              $(SRC_CPU_DIR)/complex.c \
              $(SRC_CPU_DIR)/kspace.c \
              $(SRC_CPU_DIR)/vector.c

# CPU-only implementations (replaced by .cu equivalents in GPU builds)
CPU_ONLY_SRCS = $(SRC_CPU_DIR)/nufft.c \
                $(SRC_CPU_DIR)/solver.c

# All sources for CPU build
CPU_SRCS = $(COMMON_SRCS) $(CPU_ONLY_SRCS)

# CUDA sources replace the CPU-only implementations
CUDA_SRCS = src/cuda/nufft.cu \
            src/cuda/solver.cu \
            src/cuda/kernels.cu

# Optimized CUDA sources
CUDA_OPT_SRCS =	src/cuda/optimized/nufft_opt.cu \
		src/cuda/optimized/kernels_opt.cu \
		src/cuda/solver.cu

CFLAGS_DBG  = -g3 -O0 -Wall -Wextra -Iinclude -DDEBUG
NVFLAGS_DBG = -g -G -O0 -arch=sm_86 -Iinclude -lineinfo

CFLAGS_DBG  += $(HDF5_CFLAGS)
NVFLAGS_DBG += $(HDF5_CFLAGS)

.PHONY: all cpu cuda cuda_opt clean

all: cpu cuda cuda_opt

cpu:
	mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) $(CPU_SRCS) -o $(OUT_DIR)/mri_recon_cpu $(HDF5_LINK) -lm

cuda:
	mkdir -p $(OUT_DIR)
	$(NVCC) $(NVFLAGS) $(COMMON_SRCS) $(CUDA_SRCS) -o $(OUT_DIR)/mri_recon_cuda $(HDF5_LINK) -lm

cuda_opt:
	mkdir -p $(OUT_DIR)
	$(NVCC) $(NVFLAGS) $(COMMON_SRCS) $(CUDA_OPT_SRCS) \
	-o $(OUT_DIR)/mri_recon_cuda_opt $(HDF5_LINK) -lm

debug_cpu:
	mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS_DBG) $(CPU_SRCS) -o $(OUT_DIR)/mri_recon_cpu_dbg $(HDF5_LINK) -lm

debug_cuda:
	mkdir -p $(OUT_DIR)
	$(NVCC) $(NVFLAGS_DBG) $(COMMON_SRCS) $(CUDA_SRCS) -o $(OUT_DIR)/mri_recon_gpu_dbg \
	    $(HDF5_LINK) -lm

clean:
	rm -f $(OUT_DIR)/mri_recon_cpu $(OUT_DIR)/mri_recon_cuda $(OUT_DIR)/mri_recon_cuda_opt \
	      $(OUT_DIR)/mri_recon_cpu_dbg $(OUT_DIR)/mri_recon_gpu_dbg
