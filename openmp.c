/**
 * GAUSSIAN ELIMINATION - PHIÊN BẢN OPENMP
 * Giải hệ phương trình tuyến tính với shared memory parallelism
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

// Cấu trúc lưu trữ hệ phương trình
typedef struct {
    double **A;     // Ma trận hệ số n x n
    double *b;      // Vector hằng số
    double *x;      // Vector nghiệm
    int n;          // Kích thước ma trận
} LinearSystem;

/**
 * Tạo hệ phương trình mới với kích thước n x n
 */
LinearSystem* create_system(int n) {
    LinearSystem *sys = malloc(sizeof(LinearSystem));
    sys->n = n;
    
    sys->A = malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        sys->A[i] = malloc(n * sizeof(double));
    }
    
    sys->b = malloc(n * sizeof(double));
    sys->x = malloc(n * sizeof(double));
    
    return sys;
}

/**
 * Giải phóng bộ nhớ
 */
void free_system(LinearSystem *sys) {
    if (!sys) return;
    
    for (int i = 0; i < sys->n; i++) {
        free(sys->A[i]);
    }
    free(sys->A);
    free(sys->b);
    free(sys->x);
    free(sys);
}

/**
 * Tạo hệ phương trình test với ma trận dominant diagonal
 */
void generate_test_system(LinearSystem *sys) {
    int n = sys->n;
    
    // Tạo ma trận A với đường chéo chính lớn (đảm bảo khả nghịch)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                sys->A[i][j] = n + 10.0;  // Đường chéo chính lớn
            } else {
                sys->A[i][j] = 1.0 / (i + j + 1.0);  // Phần tử khác nhỏ
            }
        }
    }
    
    // Tạo vector nghiệm x cố định: x[i] = i + 1
    double *true_x = malloc(n * sizeof(double));
    for (int i = 0; i < n; i++) {
        true_x[i] = i + 1.0;
    }
    
    // Tính b = A * x
    for (int i = 0; i < n; i++) {
        sys->b[i] = 0.0;
        for (int j = 0; j < n; j++) {
            sys->b[i] += sys->A[i][j] * true_x[j];
        }
    }
    
    free(true_x);
}

/**
 * Kiểm tra tính đúng đắn nghiệm bằng cách tính A*x so với b
 */
int verify_solution(LinearSystem *sys) {
    int n = sys->n;
    double tolerance = 1e-9;
    
    for (int i = 0; i < n; i++) {
        double sum = 0.0;
        for (int j = 0; j < n; j++) {
            sum += sys->A[i][j] * sys->x[j];
        }
        
        if (fabs(sum - sys->b[i]) > tolerance) {
            return 0;  // Nghiệm không chính xác
        }
    }
    
    return 1;  // Nghiệm chính xác
}

/**
 * Thuật toán Gaussian Elimination với OpenMP
 * Song song hóa vòng lặp khử xuôi
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
            printf("Lỗi: Ma trận không khả nghịch (pivot ≈ 0)\n");
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
            
            // Cập nhật hàng i
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
    
    return 1;  // Thành công
}

/**
 * In ma trận (chỉ khi n <= 10)
 */
void print_matrix(LinearSystem *sys) {
    int n = sys->n;
    if (n > 10) return;
    
    printf("Ma trận A:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%8.2f ", sys->A[i][j]);
        }
        printf("\n");
    }
}

/**
 * In vector (chỉ khi n <= 10)
 */
void print_vector(double *v, int n, const char *name) {
    if (n > 10) return;
    
    printf("%s: ", name);
    for (int i = 0; i < n; i++) {
        printf("%.2f ", v[i]);
    }
    printf("\n");
}

/**
 * Chương trình chính
 */
int main(int argc, char *argv[]) {
    int n = 100;          // Kích thước mặc định
    int num_threads = 4;  // Số luồng mặc định
    
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
    
    printf("🧮 GAUSSIAN ELIMINATION - PHIÊN BẢN OPENMP\n");
    printf("Kích thước ma trận: %d x %d\n", n, n);
    printf("Số luồng: %d\n", num_threads);
    printf("Số processor có sẵn: %d\n\n", omp_get_num_procs());
    
    // Tạo hệ phương trình
    LinearSystem *sys = create_system(n);
    generate_test_system(sys);
    
    // Hiển thị ma trận nếu nhỏ
    if (n <= 10) {
        print_matrix(sys);
        print_vector(sys->b, n, "Vector b");
        printf("\n");
    }
    
    // Đo thời gian thực hiện bằng OpenMP timer
    double start_time = omp_get_wtime();
    
    int success = gaussian_elimination_openmp(sys, num_threads);
    
    double end_time = omp_get_wtime();
    double elapsed_time = end_time - start_time;
    
    // In kết quả
    if (success) {
        printf("✅ Giải thành công!\n");
        printf("⏱️  Thời gian thực hiện: %.6f giây\n", elapsed_time);
        
        if (n <= 10) {
            print_vector(sys->x, n, "Nghiệm x");
        }
        
        if (verify_solution(sys)) {
            printf("✅ Nghiệm chính xác!\n");
        } else {
            printf("❌ Nghiệm không chính xác!\n");
        }
        
        printf("\n📊 Thông tin hiệu năng:\n");
        printf("   - Số luồng: %d\n", num_threads);
        printf("   - Thời gian: %.6f giây\n", elapsed_time);
        
    } else {
        printf("❌ Không thể giải hệ phương trình!\n");
    }
    
    // Dọn dẹp bộ nhớ
    free_system(sys);
    
    return success ? 0 : 1;
} 