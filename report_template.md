# BÁO CÁO KHẢO SÁT HIỆU NĂNG - GAUSSIAN ELIMINATION

## 1. THÔNG TIN CHUNG

- **Tên sinh viên**: [Điền tên]
- **Mã số sinh viên**: [Điền MSSV]
- **Lớp**: [Điền lớp]
- **Ngày thực hiện**: [Điền ngày]
- **Môi trường test**: [Hệ điều hành, CPU, RAM]

## 2. MỤC TIÊU

Khảo sát và so sánh hiệu năng của các mô hình lập trình song song (OpenMP, Pthreads, MPI) trong việc giải hệ phương trình tuyến tính bằng thuật toán Gaussian Elimination.

## 3. PHÂN TÍCH LÝ THUYẾT

### 3.1 Thuật toán Gaussian Elimination

**Mô tả thuật toán**:

- Giai đoạn 1: Forward Elimination (Khử xuôi)
- Giai đoạn 2: Backward Substitution (Thế ngược)

**Độ phức tạp tính toán**: O(n³)

**Khả năng song song hóa**:

- Forward Elimination: Có thể song song hóa từng bước (mỗi bước phụ thuộc kết quả bước trước)
- Backward Substitution: Khó song song hóa do phụ thuộc dữ liệu tuần tự

### 3.2 Các mô hình song song

#### OpenMP (Open Multi-Processing)

- **Mô hình**: Shared Memory Parallel Programming
- **Ưu điểm**: Dễ sử dụng, syntax đơn giản
- **Nhược điểm**: Chỉ hoạt động trên shared memory systems

#### Pthreads (POSIX Threads)

- **Mô hình**: Manual thread management
- **Ưu điểm**: Kiểm soát chi tiết, linh hoạt cao
- **Nhược điểm**: Phức tạp hơn, dễ gây lỗi

#### MPI (Message Passing Interface)

- **Mô hình**: Distributed Memory Parallel Programming
- **Ưu điểm**: Scalable, hoạt động trên cluster
- **Nhược điểm**: Communication overhead cao

### 3.3 Speedup và Efficiency lý thuyết

**Định nghĩa**:

- Speedup(p) = T_sequential / T_parallel(p)
- Efficiency(p) = Speedup(p) / p

**Giới hạn lý thuyết**:

- Theo định luật Amdahl: Speedup ≤ 1 / (f + (1-f)/p)
- Với f là tỷ lệ phần tuần tự không thể song song hóa

## 4. THIẾT KẾ THỰC NGHIỆM

### 4.1 Môi trường thực nghiệm

**Cấu hình phần cứng**:

- CPU: [Điền thông tin CPU]
- RAM: [Điền thông tin RAM]
- Số cores: [Điền số cores]

**Cấu hình phần mềm**:

- Hệ điều hành: [Điền OS]
- Compiler: [Điền compiler version]
- OpenMP version: [Điền version]
- MPI implementation: [Điền implementation]

### 4.2 Tham số thực nghiệm

**Kích thước ma trận**: N = [100, 200, 500, 1000, 2000] (tùy chọn)

**Số luồng/processes**: p = [3, 5, 7, 9, 11]

**Số lần chạy**: [Điền số lần] lần cho mỗi cấu hình (lấy trung bình)

## 5. KẾT QUẢ THỰC NGHIỆM

### 5.1 Bảng kết quả tổng hợp

#### Ma trận 1000x1000:

| Phương pháp | Số luồng | Thời gian (s) | Speedup | Efficiency |
| ----------- | -------- | ------------- | ------- | ---------- |
| Sequential  | 1        | [điền]        | 1.00    | 1.00       |
| OpenMP      | 3        | [điền]        | [điền]  | [điền]     |
| OpenMP      | 5        | [điền]        | [điền]  | [điền]     |
| OpenMP      | 7        | [điền]        | [điền]  | [điền]     |
| OpenMP      | 9        | [điền]        | [điền]  | [điền]     |
| OpenMP      | 11       | [điền]        | [điền]  | [điền]     |
| Pthread     | 3        | [điền]        | [điền]  | [điền]     |
| Pthread     | 5        | [điền]        | [điền]  | [điền]     |
| Pthread     | 7        | [điền]        | [điền]  | [điền]     |
| Pthread     | 9        | [điền]        | [điền]  | [điền]     |
| Pthread     | 11       | [điền]        | [điền]  | [điền]     |
| MPI         | 3        | [điền]        | [điền]  | [điền]     |
| MPI         | 5        | [điền]        | [điền]  | [điền]     |
| MPI         | 7        | [điền]        | [điền]  | [điền]     |
| MPI         | 9        | [điền]        | [điền]  | [điền]     |
| MPI         | 11       | [điền]        | [điền]  | [điền]     |

_Ghi chú: Có thể tạo thêm bảng cho các kích thước ma trận khác_

### 5.2 Biểu đồ

**[Chèn biểu đồ Speedup vs Số luồng]**

- Trục X: Số luồng (3, 5, 7, 9, 11)
- Trục Y: Speedup
- 3 đường: OpenMP, Pthread, MPI

**[Chèn biểu đồ Efficiency vs Số luồng]**

- Trục X: Số luồng (3, 5, 7, 9, 11)
- Trục Y: Efficiency
- 3 đường: OpenMP, Pthread, MPI

**[Chèn biểu đồ Thời gian vs Kích thước ma trận]**

- So sánh các phương pháp với số luồng cố định

## 6. PHÂN TÍCH VÀ THẢO LUẬN

### 6.1 So sánh hiệu năng

**OpenMP vs Sequential**:

- Speedup tốt nhất: [điền] lần với [điền] luồng
- Efficiency cao nhất: [điền]% với [điền] luồng
- Phân tích nguyên nhân: [Viết phân tích]

**Pthread vs Sequential**:

- Speedup tốt nhất: [điền] lần với [điền] luồng
- Efficiency cao nhất: [điền]% với [điền] luồng
- Phân tích nguyên nhân: [Viết phân tích]

**MPI vs Sequential**:

- Speedup tốt nhất: [điền] lần với [điền] processes
- Efficiency cao nhất: [điền]% với [điền] processes
- Phân tích nguyên nhân: [Viết phân tích]

### 6.2 So sánh giữa các mô hình song song

**OpenMP vs Pthread**:

- [Viết so sánh về speedup, efficiency, ease of use]

**OpenMP vs MPI**:

- [Viết so sánh về scalability, communication overhead]

**Pthread vs MPI**:

- [Viết so sánh về flexibility, complexity]

### 6.3 Phân tích các yếu tố ảnh hưởng

**Load balancing**:

- [Phân tích cách phân chia công việc ảnh hưởng đến hiệu năng]

**Communication overhead**:

- [Phân tích chi phí giao tiếp trong MPI]

**Synchronization overhead**:

- [Phân tích chi phí đồng bộ trong OpenMP và Pthread]

**Memory access patterns**:

- [Phân tích pattern truy cập bộ nhớ]

### 6.4 Giới hạn của thực nghiệm

- [Liệt kê các giới hạn và cách khắc phục]

## 7. KẾT LUẬN

### 7.1 Tóm tắt kết quả

- **Phương pháp tốt nhất**: [Điền phương pháp] với [điền lý do]
- **Speedup cao nhất đạt được**: [điền] lần
- **Efficiency cao nhất**: [điền]%

### 7.2 Bài học kinh nghiệm

- **OpenMP**: [Điền nhận xét]
- **Pthread**: [Điền nhận xét]
- **MPI**: [Điền nhận xét]

### 7.3 Ứng dụng thực tế

- [Viết về các ứng dụng của parallel computing trong thực tế]

## 8. ĐỀ XUẤT CẢI TIẾN

### 8.1 Cải tiến thuật toán

- **Block-based algorithms**: Sử dụng LU decomposition với blocking
- **SIMD instructions**: Tận dụng vector instructions
- **GPU acceleration**: Sử dụng CUDA hoặc OpenCL

### 8.2 Cải tiến implementation

- **Load balancing**: Dynamic work distribution
- **Memory optimization**: Cache-friendly memory access
- **Hybrid approaches**: Kết hợp MPI + OpenMP

### 8.3 Nghiên cứu tiếp theo

- Test trên cluster lớn hơn
- So sánh với các thư viện tối ưu (LAPACK, BLAS)
- Implement trên GPU

## 9. TÀI LIỆU THAM KHẢO

1. [Thêm các tài liệu đã tham khảo]
2. ...

## PHỤ LỤC

### Phụ lục A: Source code chính

- [Có thể attach hoặc link đến source code]

### Phụ lục B: Log kết quả chi tiết

- [Attach file performance_results.csv]

### Phụ lục C: Script và Makefile

- [Attach run_tests.sh và Makefile]

---

**Ghi chú**:

- Điền tất cả các phần có dấu [điền] hoặc [Viết ...]
- Thêm biểu đồ thực tế từ kết quả test
- Có thể điều chỉnh cấu trúc theo yêu cầu cụ thể của giảng viên
