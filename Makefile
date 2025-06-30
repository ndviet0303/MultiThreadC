# Makefile cho Gaussian Elimination - Tất cả phiên bản
# Hỗ trợ Sequential, OpenMP, Pthreads, MPI

# Compiler và flags
CC = gcc
MPICC = mpicc
CFLAGS = -Wall -O3 -std=c99
# Detect if we're using clang (macOS) or gcc (Linux)
OPENMP_FLAGS = $(shell if $(CC) --version 2>&1 | grep -q clang; then echo "-Xpreprocessor -fopenmp -lomp"; else echo "-fopenmp"; fi)
PTHREAD_FLAGS = -lpthread
MPI_FLAGS = 
MATH_FLAGS = -lm

# Thư mục và file
SRC_DIR = .
BUILD_DIR = build
UTILS_SRC = utils.c
UTILS_OBJ = $(BUILD_DIR)/utils.o

# Danh sách các target chính
EXECUTABLES = main_demo sequential openmp_version pthread_version mpi_version performance_test

.PHONY: all clean help test install setup main_demo

# Disable implicit rules
.SUFFIXES:

# Build tất cả
all: $(BUILD_DIR) $(EXECUTABLES)

# Tạo thư mục build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build utils.o (dùng chung cho tất cả)
$(UTILS_OBJ): $(UTILS_SRC) utils.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $(UTILS_SRC) -o $(UTILS_OBJ) $(MATH_FLAGS)

# Build main demo
main_demo: $(BUILD_DIR)/main_demo
$(BUILD_DIR)/main_demo: main.c $(UTILS_OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ main.c $(UTILS_OBJ) $(MATH_FLAGS)

# Build phiên bản tuần tự
sequential: $(BUILD_DIR)/sequential
$(BUILD_DIR)/sequential: sequential.c $(UTILS_OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ sequential.c $(UTILS_OBJ) $(MATH_FLAGS)

# Build phiên bản OpenMP
openmp_version: $(BUILD_DIR)/openmp_version
$(BUILD_DIR)/openmp_version: openmp_version.c $(UTILS_OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OPENMP_FLAGS) -o $@ openmp_version.c $(UTILS_OBJ) $(MATH_FLAGS)

# Build phiên bản Pthread
pthread_version: $(BUILD_DIR)/pthread_version
$(BUILD_DIR)/pthread_version: pthread_version.c $(UTILS_OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ pthread_version.c $(UTILS_OBJ) $(PTHREAD_FLAGS) $(MATH_FLAGS)

# Build phiên bản MPI
mpi_version: $(BUILD_DIR)/mpi_version
$(BUILD_DIR)/mpi_version: mpi_version.c $(UTILS_OBJ) | $(BUILD_DIR)
	$(MPICC) $(CFLAGS) $(MPI_FLAGS) -o $@ mpi_version.c $(UTILS_OBJ) $(MATH_FLAGS)

# Build chương trình test hiệu năng (cần objects từ sequential, openmp, pthread)
$(BUILD_DIR)/sequential_lib.o: sequential.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c sequential.c -o $@ $(MATH_FLAGS) -DLIB_MODE

$(BUILD_DIR)/openmp_lib.o: openmp_version.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OPENMP_FLAGS) -c openmp_version.c -o $@ $(MATH_FLAGS) -DLIB_MODE

$(BUILD_DIR)/pthread_lib.o: pthread_version.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c pthread_version.c -o $@ $(MATH_FLAGS) -DLIB_MODE

performance_test: $(BUILD_DIR)/performance_test
$(BUILD_DIR)/performance_test: performance_test.c $(UTILS_OBJ) $(BUILD_DIR)/sequential_lib.o $(BUILD_DIR)/openmp_lib.o $(BUILD_DIR)/pthread_lib.o | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OPENMP_FLAGS) -o $@ performance_test.c $(UTILS_OBJ) $(BUILD_DIR)/sequential_lib.o $(BUILD_DIR)/openmp_lib.o $(BUILD_DIR)/pthread_lib.o $(PTHREAD_FLAGS) $(MATH_FLAGS)

# Chạy test đơn giản
test: all
	@echo "=== Running Main Demo ==="
	./$(BUILD_DIR)/main_demo
	@echo ""
	@echo "=== Testing Sequential Version ==="
	./$(BUILD_DIR)/sequential 10
	@echo ""
	@echo "=== Testing OpenMP Version ==="
	./$(BUILD_DIR)/openmp_version 10 4
	@echo ""
	@echo "=== Testing Pthread Version ==="
	./$(BUILD_DIR)/pthread_version 10 4

# Chạy test hiệu năng với kích thước nhỏ
test-performance: $(BUILD_DIR)/performance_test
	@echo "=== Running Performance Test ==="
	./$(BUILD_DIR)/performance_test 200

# Chạy test MPI (cần mpirun)
test-mpi: $(BUILD_DIR)/mpi_version
	@echo "=== Testing MPI Version ==="
	mpirun -np 4 ./$(BUILD_DIR)/mpi_version 100

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
	rm -f performance_results.csv
	rm -f *.o core

# Hiển thị trợ giúp
help:
	@echo "Available targets:"
	@echo "  all                 - Build tất cả phiên bản"
	@echo "  main_demo           - Build chương trình demo chính"
	@echo "  sequential          - Build phiên bản tuần tự"
	@echo "  openmp_version      - Build phiên bản OpenMP"
	@echo "  pthread_version     - Build phiên bản Pthread"
	@echo "  mpi_version         - Build phiên bản MPI"
	@echo "  performance_test    - Build chương trình test hiệu năng"
	@echo "  test               - Chạy test cơ bản"
	@echo "  test-performance   - Chạy test hiệu năng"
	@echo "  test-mpi          - Chạy test MPI"
	@echo "  clean             - Xóa file build"
	@echo "  help              - Hiển thị trợ giúp này"
	@echo ""
	@echo "Usage examples:"
	@echo "  make all                                    # Build tất cả"
	@echo "  ./build/sequential 1000                    # Chạy sequential với n=1000"
	@echo "  ./build/openmp_version 1000 8              # OpenMP với 8 threads"
	@echo "  ./build/pthread_version 1000 4             # Pthread với 4 threads"
	@echo "  mpirun -np 4 ./build/mpi_version 1000      # MPI với 4 processes"
	@echo "  ./build/performance_test 500               # Test hiệu năng với n=500"

# Thiết lập quyền execute cho script
setup:
	chmod +x run_tests.sh
	@echo "✅ Setup completed! Script executable."

# Kiểm tra dependencies
check-deps:
	@echo "Checking dependencies..."
	@which gcc > /dev/null || (echo "ERROR: gcc not found"; exit 1)
	@which mpicc > /dev/null || (echo "WARNING: mpicc not found, MPI version will not work")
	@echo "Dependencies OK"

# Install (copy executables to system path - optional)
install: all
	@echo "Installing to /usr/local/bin (requires sudo)..."
	sudo cp $(BUILD_DIR)/sequential /usr/local/bin/gaussian-sequential
	sudo cp $(BUILD_DIR)/openmp_version /usr/local/bin/gaussian-openmp
	sudo cp $(BUILD_DIR)/pthread_version /usr/local/bin/gaussian-pthread
	sudo cp $(BUILD_DIR)/performance_test /usr/local/bin/gaussian-performance
	@echo "Installation complete!"

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: all
	@echo "Debug build completed" 