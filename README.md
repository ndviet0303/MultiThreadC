# 🧮 Gaussian Elimination - Parallel Computing

Bộ 4 file tối giản triển khai thuật toán **Gaussian Elimination** giải hệ phương trình tuyến tính với các mô hình song song khác nhau.

## 📂 Cấu trúc dự án

```
MultiThreadC/
├── sequential.c     # Phiên bản tuần tự (baseline)
├── openmp.c        # Song song OpenMP (shared memory)
├── pthread.c       # Song song Pthread (manual threading)
├── mpi.c          # Song song MPI (distributed memory)
├── Makefile       # Build script thông minh
├── README.md      # Hướng dẫn này
└── build/         # Thư mục chứa file executable
    ├── sequential
    ├── openmp
    ├── pthread
    └── mpi
```

## ⚡ Bắt đầu nhanh

### 1. Biên dịch tất cả

```bash
make all
```

### 2. Chạy test cơ bản

```bash
# Tuần tự (baseline)
build/sequential 100

# OpenMP - 4 luồng
build/openmp 100 4

# Pthread - 4 luồng  
build/pthread 100 4

# MPI - 4 processes
mpirun -np 4 build/mpi 100
```

### 3. Test hiệu năng

```bash
make test-performance
```

## 🧪 Thuật toán

**Gaussian Elimination** với **Partial Pivoting**:

1. **Forward Elimination (Khử xuôi)**
   - Tìm pivot lớn nhất trong cột hiện tại
   - Hoán đổi hàng để đưa pivot lên vị trí đường chéo
   - Khử tất cả phần tử dưới pivot về 0

2. **Backward Substitution (Thế ngược)**
   - Giải từ phương trình cuối cùng lên đầu
   - Thế ngược để tìm nghiệm hoàn chỉnh

**Độ phức tạp**: O(n³) cho tất cả phiên bản

## 🚀 Chiến lược song song

### 🔸 Sequential (`sequential.c`)
- **Mô hình**: Tuần tự hoàn toàn
- **Mục đích**: Baseline để so sánh hiệu năng
- **Ưu điểm**: Đơn giản, dễ hiểu, ít lỗi
- **Nhược điểm**: Chậm với ma trận lớn

### 🔸 OpenMP (`openmp.c`)
- **Mô hình**: Shared memory parallelism
- **Kỹ thuật**: `#pragma omp parallel for` 
- **Song song hóa**: Vòng lặp khử xuôi
- **Ưu điểm**: Dễ code, hiệu quả cao
- **Nhược điểm**: Giới hạn trong 1 máy

### 🔸 Pthread (`pthread.c`)
- **Mô hình**: Manual thread management
- **Kỹ thuật**: Thread pool + mutex synchronization
- **Song song hóa**: Tìm pivot và khử hàng
- **Ưu điểm**: Kiểm soát chi tiết
- **Nhược điểm**: Phức tạp, dễ deadlock

### 🔸 MPI (`mpi.c`)
- **Mô hình**: Distributed memory parallelism
- **Kỹ thuật**: Row distribution + collective communication
- **Song song hóa**: Phân phối hàng cho processes
- **Ưu điểm**: Mở rộng nhiều máy
- **Nhược điểm**: Overhead communication

## 📊 So sánh hiệu năng

| Mô hình | Kiến trúc | Speedup lý thuyết | Phù hợp |
|---------|-----------|-------------------|---------|
| Sequential | Single-threaded | 1.0x | Ma trận nhỏ |
| OpenMP | Multi-threaded | 2-8x | Workstation |
| Pthread | Multi-threaded | 2-6x | Embedded systems |
| MPI | Multi-process | 2-Nx | HPC clusters |

## 🛠️ Yêu cầu hệ thống

### Bắt buộc:
- **GCC** với OpenMP support
- **POSIX Threads** (pthread)
- **Make** build tool

### Tùy chọn (cho MPI):
- **OpenMPI** hoặc **MPICH**

### Cài đặt:

**macOS:**
```bash
# Cài đặt dependencies cho OpenMP và MPI
brew install gcc libomp open-mpi

# Kiểm tra gcc có sẵn
which gcc-13 || which gcc-12
```

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential libgomp1 libopenmpi-dev openmpi-bin
```

**Lưu ý quan trọng:**
- **Sequential và Pthread**: Hoạt động trên mọi hệ thống có GCC
- **OpenMP**: Cần homebrew GCC trên macOS, system GCC trên Linux
- **MPI**: Cần cài đặt OpenMPI hoặc MPICH

## 📈 Test hiệu năng chi tiết

### Test với các số luồng khác nhau:

```bash
# OpenMP scalability
for threads in 1 2 4 6 8; do
    echo "=== OpenMP $threads threads ==="
    time build/openmp 1000 $threads
done

# Pthread scalability  
for threads in 1 2 4 6 8; do
    echo "=== Pthread $threads threads ==="
    time build/pthread 1000 $threads
done

# MPI scalability
for procs in 1 2 4 6 8; do
    echo "=== MPI $procs processes ==="
    time mpirun -np $procs build/mpi 1000
done
```

### Đo speedup tự động:

```bash
#!/bin/bash
n=1000

# Baseline
echo "Measuring baseline..."
seq_time=$(build/sequential $n | grep "Thời gian" | awk '{print $4}')

# OpenMP speedup
omp_time=$(build/openmp $n 4 | grep "Thời gian" | awk '{print $4}')
omp_speedup=$(echo "scale=2; $seq_time / $omp_time" | bc)

echo "Sequential: ${seq_time}s"
echo "OpenMP:     ${omp_time}s (${omp_speedup}x speedup)"
```

## 🎯 Kết quả mẫu

```
=== PERFORMANCE TEST (n=500) ===

Sequential:
  Time: 0.234567 seconds
  
OpenMP (4 threads):  
  Time: 0.089123 seconds
  Speedup: 2.63x
  Efficiency: 65.8%
  
Pthread (4 threads):
  Time: 0.094521 seconds  
  Speedup: 2.48x
  Efficiency: 62.0%
  
MPI (4 processes):
  Time: 0.156789 seconds
  Speedup: 1.50x
  Efficiency: 37.5%
```

## 🔬 Phân tích thuật toán

### Phần song song hóa được:
- **Forward elimination**: Khử các hàng độc lập ✅
- **Pivot selection**: Tìm max có thể song song ✅

### Phần tuần tự (bottleneck):
- **Backward substitution**: Phụ thuộc tuần tự ❌
- **Row swapping**: Cần đồng bộ ❌

### Giới hạn Speedup (theo Amdahl's Law):
```
S = 1 / (f + (1-f)/p)
```
Với `f ≈ 30%` phần tuần tự → Speedup max ≈ 3.3x

## 🧮 Ma trận test

**Dominant Diagonal Matrix**: Đảm bảo khả nghịch và numerical stability

```c
// Cấu trúc ma trận
A[i][i] = n + 10.0          // Đường chéo lớn
A[i][j] = 1.0/(i+j+1.0)     // Phần tử khác nhỏ

// Nghiệm test
x[i] = i + 1.0              // [1, 2, 3, ..., n]

// Vector b
b = A * x                   // Tính từ nghiệm đã biết
```

## 🐛 Troubleshooting

### Lỗi compilation OpenMP:
```bash
# Thử compiler khác
clang -fopenmp -Xpreprocessor -fopenmp -lomp -o openmp openmp.c
```

### Lỗi MPI không tìm thấy:
```bash
# Kiểm tra MPI installation
which mpicc
which mpirun

# Set PATH nếu cần
export PATH="/usr/local/bin:$PATH"
```

### Lỗi linking pthread:
```bash
# Explicit linking
gcc -pthread -lpthread -o pthread pthread.c
```

### Ma trận singular:
```bash
# Giảm kích thước test
build/sequential 10

# Ma trận dominant diagonal rất ít khi singular
```

## 📋 Makefile targets

```bash
make help                # Hiển thị trợ giúp
make all                 # Build tất cả 
make sequential          # Build tuần tự
make openmp              # Build OpenMP
make pthread             # Build Pthread  
make mpi                 # Build MPI
make test-small          # Test ma trận 10x10
make test-performance    # Test ma trận 500x500
make clean               # Xóa thư mục build/
```

## 📚 Tài liệu tham khảo

1. **Thuật toán**: Introduction to Algorithms (CLRS) - Chapter 28
2. **OpenMP**: OpenMP Application Programming Interface
3. **Pthread**: Programming with POSIX Threads (David Butenhof)
4. **MPI**: Using MPI - Portable Parallel Programming (Gropp, Lusk, Skjellum)
5. **Parallel Algorithms**: Introduction to Parallel Algorithms (JáJá)

## 🎓 Ứng dụng thực tế

- **Scientific Computing**: Mô phỏng vật lý, hóa học
- **Machine Learning**: Linear regression, neural networks
- **Computer Graphics**: Lighting, rendering pipelines
- **Engineering**: FEM, CFD simulations
- **Economics**: Linear programming, optimization

---

**Tác giả**: Implementation for Parallel Computing Education  
**Phiên bản**: Tối giản 4-file standalone  
**Ngôn ngữ**: C with OpenMP/Pthreads/MPI  
**License**: Educational use 