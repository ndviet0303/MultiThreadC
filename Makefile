# Makefile cho 4 phiên bản Gaussian Elimination
CC = gcc
CFLAGS = -Wall -O2 -lm

# Thư mục output
BUILD_DIR = build

# OpenMP: macOS cần homebrew gcc và libomp
# Ubuntu/Linux dùng gcc system
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS: sử dụng homebrew gcc và libomp
    OPENMP_CC := $(shell which gcc-15 2>/dev/null || which gcc-14 2>/dev/null || which gcc-13 2>/dev/null || echo "clang")
    LIBOMP_PREFIX := $(shell brew --prefix libomp 2>/dev/null || echo "/opt/homebrew/opt/libomp")
    ifeq ($(OPENMP_CC),clang)
        # Fallback: clang với libomp từ homebrew
        OPENMP_FLAGS = -Xpreprocessor -fopenmp -I$(LIBOMP_PREFIX)/include -L$(LIBOMP_PREFIX)/lib -lomp
    else
        # Sử dụng homebrew gcc
        OPENMP_FLAGS = -fopenmp -I$(LIBOMP_PREFIX)/include -L$(LIBOMP_PREFIX)/lib
    endif
else
    # Linux: dùng gcc system
    OPENMP_CC = gcc
    OPENMP_FLAGS = -fopenmp
endif

# MPI compiler
MPICC = mpicc

# Tạo thư mục build
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Build tất cả
all: $(BUILD_DIR) sequential openmp pthread mpi

# Phiên bản tuần tự
sequential: $(BUILD_DIR) sequential.c
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/sequential sequential.c
	@echo "✅ Sequential build thành công → $(BUILD_DIR)/sequential"

# Phiên bản OpenMP
openmp: $(BUILD_DIR) openmp.c
	@echo "Building OpenMP version..."
	@if $(OPENMP_CC) $(CFLAGS) $(OPENMP_FLAGS) -o $(BUILD_DIR)/openmp openmp.c 2>/dev/null; then \
		echo "✅ OpenMP build thành công → $(BUILD_DIR)/openmp"; \
	else \
		echo "❌ OpenMP build thất bại"; \
		echo "💡 Cài đặt: brew install gcc libomp (macOS) hoặc apt install libgomp1 (Linux)"; \
		exit 1; \
	fi

# Phiên bản Pthread
pthread: $(BUILD_DIR) pthread.c
	$(CC) $(CFLAGS) -pthread -o $(BUILD_DIR)/pthread pthread.c
	@echo "✅ Pthread build thành công → $(BUILD_DIR)/pthread"

# Phiên bản MPI
mpi: $(BUILD_DIR) mpi.c
	@echo "Building MPI version..."
	@if command -v $(MPICC) >/dev/null 2>&1; then \
		$(MPICC) $(CFLAGS) -o $(BUILD_DIR)/mpi mpi.c && echo "✅ MPI build thành công → $(BUILD_DIR)/mpi"; \
	else \
		echo "❌ MPI compiler không tìm thấy"; \
		echo "💡 Cài đặt: brew install open-mpi (macOS) hoặc apt install libopenmpi-dev (Linux)"; \
		exit 1; \
	fi

# Test nhanh - chỉ test các phiên bản build được
test-small: sequential pthread
	@echo "=== TEST SEQUENTIAL ==="
	$(BUILD_DIR)/sequential 10
	@echo "\n=== TEST PTHREAD ==="
	$(BUILD_DIR)/pthread 10 4
	@if [ -f "$(BUILD_DIR)/openmp" ]; then \
		echo "\n=== TEST OPENMP ==="; \
		$(BUILD_DIR)/openmp 10 4; \
	else \
		echo "\n⚠️  OpenMP chưa build (cần: brew install gcc libomp)"; \
	fi
	@if [ -f "$(BUILD_DIR)/mpi" ] && command -v mpirun >/dev/null 2>&1; then \
		echo "\n=== TEST MPI ==="; \
		mpirun -np 4 $(BUILD_DIR)/mpi 10; \
	else \
		echo "\n⚠️  MPI chưa build (cần: brew install open-mpi)"; \
	fi

# Test tất cả (bỏ qua lỗi)
test-all: 
	@$(MAKE) all || true
	@$(MAKE) test-small

# Test hiệu năng
test-performance: all
	@echo "=== PERFORMANCE TEST (n=500) ==="
	@echo "Sequential:"
	@time $(BUILD_DIR)/sequential 500
	@if [ -f "$(BUILD_DIR)/openmp" ]; then \
		echo "\nOpenMP (4 threads):"; \
		time $(BUILD_DIR)/openmp 500 4; \
	fi
	@if [ -f "$(BUILD_DIR)/pthread" ]; then \
		echo "\nPthread (4 threads):"; \
		time $(BUILD_DIR)/pthread 500 4; \
	fi
	@if [ -f "$(BUILD_DIR)/mpi" ]; then \
		echo "\nMPI (4 processes):"; \
		time mpirun -np 4 $(BUILD_DIR)/mpi 500; \
	fi

# Dọn dẹp
clean:
	rm -rf $(BUILD_DIR)
	@echo "🗑️  Đã xóa thư mục $(BUILD_DIR)/"

# Trợ giúp
help:
	@echo "Gaussian Elimination - Parallel Computing"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build tất cả 4 phiên bản"
	@echo "  sequential       - Build phiên bản tuần tự"
	@echo "  openmp          - Build phiên bản OpenMP"
	@echo "  pthread         - Build phiên bản Pthread"
	@echo "  mpi             - Build phiên bản MPI"
	@echo "  test-small      - Test nhanh (10x10)"
	@echo "  test-performance - Test hiệu năng (500x500)"
	@echo "  clean           - Xóa executables"
	@echo "  help            - Hiển thị trợ giúp"
	@echo ""
	@echo "Usage:"
	@echo "  $(BUILD_DIR)/sequential [n]           - Chạy tuần tự"
	@echo "  $(BUILD_DIR)/openmp [n] [threads]     - Chạy OpenMP"
	@echo "  $(BUILD_DIR)/pthread [n] [threads]    - Chạy Pthread"
	@echo "  mpirun -np [procs] $(BUILD_DIR)/mpi [n] - Chạy MPI"
	@echo ""
	@echo "File outputs:"
	@echo "  All executables → $(BUILD_DIR)/"

.PHONY: all test-small test-performance clean help 