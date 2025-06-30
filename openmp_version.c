#include "utils.h"
#include <omp.h>

/**
 * Thuật toán Gaussian Elimination sử dụng OpenMP
 * Song song hóa các vòng lặp trong quá trình khử xuôi và thế ngược
 */
int gaussian_elimination_openmp(LinearSystem *sys, int num_threads) {
    int n = sys->n;
    double **A = sys->A;
    double *b = sys->b;
    double *x = sys->x;
    
    // Thiết lập số luồng
    omp_set_num_threads(num_threads);
    
    // Giai đoạn 1: Khử xuôi (Forward Elimination)
    for (int k = 0; k < n - 1; k++) {
        // Tìm pivot lớn nhất trong cột k (tuần tự vì cần tìm max)
        int max_row = k;
        double max_val = fabs(A[k][k]);
        
        for (int i = k + 1; i < n; i++) {
            if (fabs(A[i][k]) > max_val) {
                max_val = fabs(A[i][k]);
                max_row = i;
            }
        }
        
        // Kiểm tra ma trận có khả nghịch không
        if (max_val < 1e-12) {
            printf("Lỗi: Ma trận không khả nghịch (pivot = 0)\n");
            return 0;
        }
        
        // Hoán đổi hàng k với hàng max_row nếu cần (tuần tự)
        if (max_row != k) {
            // Hoán đổi trong ma trận A
            double *temp_row = A[k];
            A[k] = A[max_row];
            A[max_row] = temp_row;
            
            // Hoán đổi trong vector b
            double temp = b[k];
            b[k] = b[max_row];
            b[max_row] = temp;
        }
        
        // Song song hóa việc khử các phần tử dưới pivot
        #pragma omp parallel for schedule(static)
        for (int i = k + 1; i < n; i++) {
            double factor = A[i][k] / A[k][k];
            
            // Cập nhật hàng i (song song hóa vòng lặp trong)
            for (int j = k; j < n; j++) {
                A[i][j] -= factor * A[k][j];
            }
            b[i] -= factor * b[k];
        }
    }
    
    // Kiểm tra phần tử cuối cùng trên đường chéo
    if (fabs(A[n-1][n-1]) < 1e-12) {
        printf("Lỗi: Ma trận không khả nghịch\n");
        return 0;
    }
    
    // Giai đoạn 2: Thế ngược (Backward Substitution)
    // Phần này khó song song hóa do sự phụ thuộc dữ liệu
    for (int i = n - 1; i >= 0; i--) {
        x[i] = b[i];
        
        // Song song hóa phép tính tổng (nếu có đủ phần tử)
        double sum = 0.0;
        #pragma omp parallel for reduction(+:sum) if(n-i-1 > 50)
        for (int j = i + 1; j < n; j++) {
            sum += A[i][j] * x[j];
        }
        
        x[i] -= sum;
        x[i] /= A[i][i];
    }
    
    return 1; // Thành công
}

/**
 * Chương trình test phiên bản OpenMP
 */
#ifndef LIB_MODE
int main(int argc, char *argv[]) {
    int n = 100; // Kích thước mặc định
    int num_threads = 4; // Số luồng mặc định
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) {
            printf("Kích thước ma trận phải > 0\n");
            return 1;
        }
    }
    
    if (argc > 2) {
        num_threads = atoi(argv[2]);
        if (num_threads <= 0) {
            printf("Số luồng phải > 0\n");
            return 1;
        }
    }
    
    printf("=== PHIÊN BẢN OPENMP - GAUSSIAN ELIMINATION ===\n");
    printf("Kích thước ma trận: %d x %d\n", n, n);
    printf("Số luồng: %d\n", num_threads);
    printf("Số processor có sẵn: %d\n\n", omp_get_num_procs());
    
    // Tạo hệ phương trình
    LinearSystem *sys = create_system(n);
    generate_test_system(sys);
    
    // Hiển thị ma trận nếu nhỏ
    if (n <= 10) {
        print_matrix(sys->A, n);
        printf("Vector b: ");
        print_vector(sys->b, n);
        printf("\n");
    }
    
    // Đo thời gian giải bằng OpenMP timer
    double start_time = omp_get_wtime();
    
    int success = gaussian_elimination_openmp(sys, num_threads);
    
    double end_time = omp_get_wtime();
    double elapsed_time = end_time - start_time;
    
    if (success) {
        printf("✅ Giải thành công!\n");
        printf("⏱️  Thời gian thực hiện: %.6f giây\n", elapsed_time);
        
        // Hiển thị nghiệm nếu ma trận nhỏ
        if (n <= 10) {
            printf("Nghiệm x: ");
            print_vector(sys->x, n);
        }
        
        // Kiểm tra tính đúng đắn của nghiệm
        if (verify_solution(sys)) {
            printf("✅ Nghiệm chính xác!\n");
        } else {
            printf("❌ Nghiệm không chính xác!\n");
        }
        
        // Thông tin về hiệu năng
        printf("\n📊 Thông tin hiệu năng:\n");
        printf("   - Số luồng đã sử dụng: %d\n", num_threads);
        printf("   - Thời gian: %.6f giây\n", elapsed_time);
        
    } else {
        printf("❌ Không thể giải hệ phương trình!\n");
    }
    
    // Dọn dẹp bộ nhớ
    free_system(sys);
    
    return 0;
}
#endif // LIB_MODE 