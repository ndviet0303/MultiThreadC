#include "utils.h"

/**
 * Tạo hệ phương trình mới với kích thước n x n
 */
LinearSystem* create_system(int n) {
    LinearSystem *sys = (LinearSystem*)malloc(sizeof(LinearSystem));
    sys->n = n;
    
    // Cấp phát bộ nhớ cho ma trận A
    sys->A = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        sys->A[i] = (double*)malloc(n * sizeof(double));
    }
    
    // Cấp phát bộ nhớ cho các vector
    sys->b = (double*)malloc(n * sizeof(double));
    sys->x = (double*)malloc(n * sizeof(double));
    
    return sys;
}

/**
 * Giải phóng bộ nhớ của hệ phương trình
 */
void free_system(LinearSystem *sys) {
    if (sys == NULL) return;
    
    for (int i = 0; i < sys->n; i++) {
        free(sys->A[i]);
    }
    free(sys->A);
    free(sys->b);
    free(sys->x);
    free(sys);
}

/**
 * Tạo hệ phương trình test với nghiệm đã biết
 * Tạo ma trận A khả nghịch và vector x ngẫu nhiên, sau đó tính b = A*x
 */
void generate_test_system(LinearSystem *sys) {
    srand(time(NULL));
    int n = sys->n;
    
    // Tạo ma trận A ngẫu nhiên với đường chéo chính lớn (đảm bảo khả nghịch)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                sys->A[i][j] = (double)(rand() % 10 + 10); // Đường chéo từ 10-19
            } else {
                sys->A[i][j] = (double)(rand() % 10 - 5); // Phần tử khác từ -5 đến 4
            }
        }
    }
    
    // Tạo vector nghiệm x ngẫu nhiên
    double *true_x = (double*)malloc(n * sizeof(double));
    for (int i = 0; i < n; i++) {
        true_x[i] = (double)(rand() % 20 - 10); // Từ -10 đến 9
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
 * In ma trận (chỉ in khi n <= 10 để tránh spam)
 */
void print_matrix(double **A, int n) {
    if (n > 10) {
        printf("Ma trận quá lớn để hiển thị (n=%d)\n", n);
        return;
    }
    
    printf("Ma trận A:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%8.2f ", A[i][j]);
        }
        printf("\n");
    }
}

/**
 * In vector (chỉ in khi n <= 10)
 */
void print_vector(double *v, int n) {
    if (n > 10) {
        printf("Vector quá lớn để hiển thị (n=%d)\n", n);
        return;
    }
    
    printf("Vector: ");
    for (int i = 0; i < n; i++) {
        printf("%8.2f ", v[i]);
    }
    printf("\n");
}

/**
 * Kiểm tra tính đúng đắn của nghiệm bằng cách tính A*x và so sánh với b
 */
int verify_solution(LinearSystem *sys) {
    int n = sys->n;
    double tolerance = 1e-6;
    
    for (int i = 0; i < n; i++) {
        double sum = 0.0;
        for (int j = 0; j < n; j++) {
            sum += sys->A[i][j] * sys->x[j];
        }
        
        if (fabs(sum - sys->b[i]) > tolerance) {
            printf("Lỗi: Nghiệm không chính xác tại hàng %d: %f != %f\n", 
                   i, sum, sys->b[i]);
            return 0;
        }
    }
    
    return 1;
}

/**
 * Tính khoảng thời gian giữa 2 timespec (đơn vị: giây)
 */
double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

/**
 * Copy hệ phương trình từ src sang dest
 */
void copy_system(LinearSystem *src, LinearSystem *dest) {
    int n = src->n;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dest->A[i][j] = src->A[i][j];
        }
        dest->b[i] = src->b[i];
        dest->x[i] = 0.0; // Reset nghiệm
    }
}

/**
 * Thuật toán Gaussian Elimination tuần tự
 * Phương pháp khử Gauss với pivoting một phần
 */
int gaussian_elimination_sequential(LinearSystem *sys) {
    int n = sys->n;
    double **A = sys->A;
    double *b = sys->b;
    double *x = sys->x;
    
    // Giai đoạn 1: Khử xuôi (Forward Elimination)
    for (int k = 0; k < n - 1; k++) {
        // Tìm pivot lớn nhất trong cột k (từ hàng k trở xuống)
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
        
        // Hoán đổi hàng k với hàng max_row nếu cần
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
        
        // Khử các phần tử dưới pivot
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
    for (int i = n - 1; i >= 0; i--) {
        x[i] = b[i];
        
        // Trừ đi các phần tử đã biết
        for (int j = i + 1; j < n; j++) {
            x[i] -= A[i][j] * x[j];
        }
        
        // Chia cho hệ số của ẩn x[i]
        x[i] /= A[i][i];
    }
    
    return 1; // Thành công
} 