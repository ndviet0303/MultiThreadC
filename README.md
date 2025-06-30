# 🧮 Gaussian Elimination - Parallel Computing Implementation

Dự án triển khai thuật toán **Gaussian Elimination** để giải hệ phương trình tuyến tính với nhiều mô hình song song khác nhau.

## 📋 Tổng quan

Dự án này bao gồm 4 phiên bản triển khai:

1. **Sequential** - Phiên bản tuần tự làm baseline
2. **OpenMP** - Song song hóa với shared memory
3. **Pthreads** - Song song hóa với manual thread management
4. **MPI** - Song song hóa với distributed memory

## 🏗️ Cấu trúc dự án

```
MultiThreadC/
├── utils.h                 # Header file chung
├── utils.c                 # Hàm tiện ích chung
├── sequential.c            # Phiên bản tuần tự
├── openmp_version.c        # Phiên bản OpenMP
├── pthread_version.c       # Phiên bản Pthreads
├── mpi_version.c           # Phiên bản MPI
├── performance_test.c      # Chương trình test hiệu năng
├── Makefile               # File build
├── run_tests.sh           # Script test tự động
├── README.md              # Hướng dẫn (file này)
└── build/                 # Thư mục chứa file binary
```

## 🛠️ Yêu cầu hệ thống

### Bắt buộc:

- **GCC** compiler với OpenMP support
- **Make** build tool
- **POSIX Threads** (pthread)

### Tùy chọn:

- **MPI** implementation (OpenMPI, MPICH) để chạy phiên bản MPI
- **bc** calculator để tính speedup trong script test

### Cài đặt trên Ubuntu/Debian:

```bash
sudo apt update
sudo apt install build-essential libgomp1 libopenmpi-dev openmpi-bin bc
```

### Cài đặt trên macOS:

```bash
brew install gcc libomp open-mpi bc
```

## 🚀 Cách sử dụng

### 1. Build tất cả phiên bản

```bash
make all
```

### 2. Chạy test cơ bản

```bash
make test
```

### 3. Chạy test tự động toàn diện

```bash
chmod +x run_tests.sh
./run_tests.sh
```

### 4. Chạy từng phiên bản riêng lẻ

#### Sequential Version:

```bash
./build/sequential [size]
# Ví dụ: ./build/sequential 1000
```

#### OpenMP Version:

```bash
./build/openmp_version [size] [num_threads]
# Ví dụ: ./build/openmp_version 1000 8
```

#### Pthread Version:

```bash
./build/pthread_version [size] [num_threads]
# Ví dụ: ./build/pthread_version 1000 4
```

#### MPI Version:

```bash
mpirun -np [num_processes] ./build/mpi_version [size]
# Ví dụ: mpirun -np 4 ./build/mpi_version 1000
```

### 5. Test hiệu năng chi tiết

```bash
./build/performance_test [size]
# Ví dụ: ./build/performance_test 500
```

## 📊 Khảo sát hiệu năng

### Script test tự động:

```bash
# Test cơ bản
./run_tests.sh --basic-only

# Test hiệu năng chi tiết
./run_tests.sh --performance-only

# Test với stress test
./run_tests.sh --with-stress
```

### Test với các số luồng khác nhau:

Theo yêu cầu, test với p = 3, 5, 7, 9, 11:

```bash
# OpenMP
for p in 3 5 7 9 11; do
    echo "Testing OpenMP with $p threads"
    ./build/openmp_version 1000 $p
done

# Pthread
for p in 3 5 7 9 11; do
    echo "Testing Pthread with $p threads"
    ./build/pthread_version 1000 $p
done

# MPI
for p in 3 5 7 9 11; do
    echo "Testing MPI with $p processes"
    mpirun -np $p ./build/mpi_version 1000
done
```

## 📈 Phân tích kết quả

### Metrics đo lường:

- **Thời gian thực hiện** (giây)
- **Speedup** = T_sequential / T_parallel
- **Efficiency** = Speedup / số_luồng
- **Tính đúng đắn** của nghiệm

### File kết quả:

- `performance_results.csv` - Kết quả chi tiết
- Console output - Kết quả realtime

### Ví dụ kết quả:

```
📊 KẾT QUẢ KHẢO SÁT HIỆU NĂNG
================================================================================
Phương pháp  Kích thước Luồng   Thời gian(s) Speedup    Efficiency
================================================================================
Sequential   1000       1        2.345672     1.00       1.00
OpenMP       1000       4        0.892341     2.63       0.66
Pthread      1000       4        0.921456     2.55       0.64
OpenMP       1000       8        0.654321     3.59       0.45
Pthread      1000       8        0.678912     3.46       0.43
```

## 🧪 Thuật toán

### Gaussian Elimination với Partial Pivoting:

1. **Forward Elimination**:

   - Tìm pivot lớn nhất trong cột hiện tại
   - Hoán đổi hàng nếu cần
   - Khử các phần tử dưới pivot về 0

2. **Backward Substitution**:
   - Giải từ phương trình cuối lên đầu
   - Thế ngược để tìm nghiệm

### Chiến lược song song hóa:

#### OpenMP:

- Song song hóa vòng lặp khử xuôi
- Sử dụng `#pragma omp parallel for`
- Reduction cho backward substitution

#### Pthreads:

- Chia hàng cho các threads
- Sử dụng barrier để đồng bộ
- Mutex cho việc tìm pivot

#### MPI:

- Phân phối hàng cho các processes
- Broadcast hàng pivot
- Gather kết quả về process 0

## 🔧 Tùy chọn Make

```bash
make all                    # Build tất cả
make sequential            # Build phiên bản tuần tự
make openmp_version        # Build phiên bản OpenMP
make pthread_version       # Build phiên bản Pthread
make mpi_version          # Build phiên bản MPI
make performance_test     # Build test hiệu năng
make test                 # Chạy test cơ bản
make test-mpi            # Test MPI
make clean               # Xóa file build
make help                # Hiển thị trợ giúp
make debug               # Build phiên bản debug
```

## 📝 Khung báo cáo

### 1. Phân tích lý thuyết

- **Độ phức tạp thuật toán**: O(n³)
- **Tính song song hóa**:
  - Forward elimination: Có thể song song hóa (phụ thuộc từng bước)
  - Backward substitution: Khó song song hóa (phụ thuộc tuần tự)
- **Speedup lý thuyết tối đa**: Giới hạn bởi phần tuần tự

### 2. Kết quả thực nghiệm

- Bảng thời gian với các giá trị p
- Biểu đồ speedup và efficiency
- So sánh giữa OpenMP, Pthread, MPI

### 3. Phân tích

- **OpenMP**: Dễ sử dụng, hiệu quả cao với shared memory
- **Pthread**: Kiểm soát chi tiết, overhead cao hơn
- **MPI**: Phù hợp distributed memory, communication overhead

### 4. Kết luận

- Đề xuất cải tiến: Block algorithms, SIMD, GPU
- Ứng dụng thực tế: Scientific computing, ML

## 🐛 Xử lý sự cố

### Lỗi compile:

```bash
# Kiểm tra dependencies
make check-deps

# Build debug version
make debug
```

### Lỗi MPI:

```bash
# Kiểm tra MPI installation
which mpicc
which mpirun

# Chạy với verbose
mpirun -v -np 4 ./build/mpi_version 10
```

### Lỗi performance:

```bash
# Giảm kích thước test
./run_tests.sh --basic-only

# Chạy với ma trận nhỏ
./build/sequential 10
```

## 📚 Tài liệu tham khảo

1. **Thuật toán**: Introduction to Algorithms (CLRS)
2. **OpenMP**: OpenMP Programming Guide
3. **MPI**: Using MPI - Portable Parallel Programming
4. **Pthread**: Programming with POSIX Threads

## 🤝 Đóng góp

Để cải tiến dự án:

1. Fork repository
2. Tạo feature branch
3. Commit changes
4. Submit pull request

## 📄 License

MIT License - Tự do sử dụng cho mục đích học tập và nghiên cứu.

---

**Tác giả**: Được tạo để minh họa parallel computing với Gaussian Elimination
**Ngày tạo**: 2024
**Môi trường test**: Linux/macOS với GCC, OpenMP, MPI
