#include "utils.h"
#include <pthread.h>

// Cấu trúc dữ liệu cho các luồng tìm pivot
typedef struct {
    LinearSystem *sys;
    int start_row;
    int end_row;
    int k;              // Cột hiện tại
    int *pivot_row;     // Kết quả: hàng có pivot max
    double *pivot_value; // Kết quả: giá trị pivot max
    pthread_mutex_t *pivot_mutex;
} PivotThreadData;

// Cấu trúc dữ liệu cho các luồng khử Gauss
typedef struct {
    LinearSystem *sys;
    int start_row;
    int end_row;
    int k;              // Bước khử hiện tại
} EliminationThreadData;

/**
 * Hàm tìm pivot lớn nhất trong phạm vi được gán
 */
void* find_pivot_thread(void* arg) {
    PivotThreadData *data = (PivotThreadData*)arg;
    LinearSystem *sys = data->sys;
    int k = data->k;
    
    int local_max_row = k;
    double local_max_val = (data->start_row <= k) ? fabs(sys->A[k][k]) : 0.0;
    
    // Tìm pivot trong phạm vi được gán
    for (int i = (data->start_row > k) ? data->start_row : k; i < data->end_row && i < sys->n; i++) {
        if (fabs(sys->A[i][k]) > local_max_val) {
            local_max_val = fabs(sys->A[i][k]);
            local_max_row = i;
        }
    }
    
    // Cập nhật pivot toàn cục với mutex
    pthread_mutex_lock(data->pivot_mutex);
    if (local_max_val > *(data->pivot_value)) {
        *(data->pivot_value) = local_max_val;
        *(data->pivot_row) = local_max_row;
    }
    pthread_mutex_unlock(data->pivot_mutex);
    
    return NULL;
}

/**
 * Hàm thực hiện khử Gauss trong phạm vi được gán
 */
void* elimination_thread(void* arg) {
    EliminationThreadData *data = (EliminationThreadData*)arg;
    LinearSystem *sys = data->sys;
    int k = data->k;
    
    // Thực hiện khử trong phạm vi được gán
    for (int i = data->start_row; i < data->end_row && i < sys->n; i++) {
        if (i > k) {  // Chỉ khử các hàng dưới pivot
            double factor = sys->A[i][k] / sys->A[k][k];
            
            // Cập nhật hàng i
            for (int j = k; j < sys->n; j++) {
                sys->A[i][j] -= factor * sys->A[k][j];
            }
            sys->b[i] -= factor * sys->b[k];
        }
    }
    
    return NULL;
}

/**
 * Thuật toán Gaussian Elimination sử dụng Pthreads với mutex và join
 */
int gaussian_elimination_pthread(LinearSystem *sys, int num_threads) {
    int n = sys->n;
    
    // Tạo mutex cho việc tìm pivot
    pthread_mutex_t pivot_mutex = PTHREAD_MUTEX_INITIALIZER;
    
    // Thực hiện khử xuôi
    for (int k = 0; k < n - 1; k++) {
        // === PHASE 1: Tìm pivot song song ===
        pthread_t *pivot_threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
        PivotThreadData *pivot_data = (PivotThreadData*)malloc(num_threads * sizeof(PivotThreadData));
        
        int pivot_row = k;
        double pivot_value = fabs(sys->A[k][k]);
        
        // Chia công việc tìm pivot cho các luồng
        int rows_per_thread = (n - k) / num_threads;
        int remaining_rows = (n - k) % num_threads;
        
        for (int i = 0; i < num_threads; i++) {
            pivot_data[i].sys = sys;
            pivot_data[i].start_row = k + i * rows_per_thread;
            pivot_data[i].end_row = k + (i + 1) * rows_per_thread;
            pivot_data[i].k = k;
            pivot_data[i].pivot_row = &pivot_row;
            pivot_data[i].pivot_value = &pivot_value;
            pivot_data[i].pivot_mutex = &pivot_mutex;
            
            // Phân bổ các hàng còn lại cho luồng cuối
            if (i == num_threads - 1) {
                pivot_data[i].end_row += remaining_rows;
            }
            
            // Tạo thread tìm pivot
            if (pthread_create(&pivot_threads[i], NULL, find_pivot_thread, &pivot_data[i]) != 0) {
                printf("Lỗi: Không thể tạo luồng tìm pivot %d\n", i);
                free(pivot_threads);
                free(pivot_data);
                return 0;
            }
        }
        
        // Chờ tất cả threads tìm pivot hoàn thành bằng pthread_join
        for (int i = 0; i < num_threads; i++) {
            pthread_join(pivot_threads[i], NULL);
        }
        
        free(pivot_threads);
        free(pivot_data);
        
        // Kiểm tra ma trận có khả nghịch không
        if (pivot_value < 1e-12) {
            printf("Lỗi: Ma trận không khả nghịch (pivot = 0 tại bước %d)\n", k);
            pthread_mutex_destroy(&pivot_mutex);
            return 0;
        }
        
        // Hoán đổi hàng nếu cần
        if (pivot_row != k) {
            // Hoán đổi trong ma trận A
            double *temp_row = sys->A[k];
            sys->A[k] = sys->A[pivot_row];
            sys->A[pivot_row] = temp_row;
            
            // Hoán đổi trong vector b
            double temp = sys->b[k];
            sys->b[k] = sys->b[pivot_row];
            sys->b[pivot_row] = temp;
        }
        
        // === PHASE 2: Khử Gauss song song ===
        pthread_t *elim_threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
        EliminationThreadData *elim_data = (EliminationThreadData*)malloc(num_threads * sizeof(EliminationThreadData));
        
        // Chia công việc khử cho các luồng
        rows_per_thread = (n - k - 1) / num_threads;
        remaining_rows = (n - k - 1) % num_threads;
        
        for (int i = 0; i < num_threads; i++) {
            elim_data[i].sys = sys;
            elim_data[i].start_row = k + 1 + i * rows_per_thread;
            elim_data[i].end_row = k + 1 + (i + 1) * rows_per_thread;
            elim_data[i].k = k;
            
            // Phân bổ các hàng còn lại cho luồng cuối
            if (i == num_threads - 1) {
                elim_data[i].end_row += remaining_rows;
            }
            
            // Chỉ tạo thread nếu có công việc
            if (elim_data[i].start_row < elim_data[i].end_row && elim_data[i].start_row < n) {
                if (pthread_create(&elim_threads[i], NULL, elimination_thread, &elim_data[i]) != 0) {
                    printf("Lỗi: Không thể tạo luồng khử %d\n", i);
                    free(elim_threads);
                    free(elim_data);
                    pthread_mutex_destroy(&pivot_mutex);
                    return 0;
                }
            } else {
                elim_threads[i] = 0; // Đánh dấu thread không được tạo
            }
        }
        
        // Chờ tất cả threads khử hoàn thành bằng pthread_join
        for (int i = 0; i < num_threads; i++) {
            if (elim_threads[i] != 0) {
                pthread_join(elim_threads[i], NULL);
            }
        }
        
        free(elim_threads);
        free(elim_data);
    }
    
    // Kiểm tra phần tử cuối cùng trên đường chéo
    if (fabs(sys->A[n-1][n-1]) < 1e-12) {
        printf("Lỗi: Ma trận không khả nghịch\n");
        pthread_mutex_destroy(&pivot_mutex);
        return 0;
    }
    
    // Giai đoạn 2: Thế ngược (tuần tự vì khó song song hóa hiệu quả)
    for (int i = n - 1; i >= 0; i--) {
        sys->x[i] = sys->b[i];
        
        for (int j = i + 1; j < n; j++) {
            sys->x[i] -= sys->A[i][j] * sys->x[j];
        }
        
        sys->x[i] /= sys->A[i][i];
    }
    
    // Dọn dẹp mutex
    pthread_mutex_destroy(&pivot_mutex);
    
    return 1; // Thành công
}

/**
 * Chương trình test phiên bản Pthreads
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
    
    printf("=== PHIÊN BẢN PTHREADS - GAUSSIAN ELIMINATION ===\n");
    printf("Kích thước ma trận: %d x %d\n", n, n);
    printf("Số luồng: %d\n\n", num_threads);
    
    // Tạo hệ phương trình test
    LinearSystem *sys = create_system(n);
    if (!sys) {
        printf("Lỗi: Không thể tạo hệ phương trình\n");
        return 1;
    }
    
    generate_test_system(sys);
    
    // Hiển thị ma trận nếu nhỏ
    if (n <= 10) {
        printf("Ma trận A:\n");
        print_matrix(sys->A, n);
        printf("Vector b: ");
        print_vector(sys->b, n);
        printf("\n");
    }
    
    // Đo thời gian thực hiện
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    int success = gaussian_elimination_pthread(sys, num_threads);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double execution_time = (end.tv_sec - start.tv_sec) + 
                           (end.tv_nsec - start.tv_nsec) / 1e9;
    
    if (success) {
        printf("✅ Giải thành công!\n");
        printf("⏱️  Thời gian thực hiện: %.6f giây\n", execution_time);
        
        if (n <= 10) {
            printf("Nghiệm x: ");
            print_vector(sys->x, n);
        }
        
        // Kiểm tra tính đúng đắn
        if (verify_solution(sys)) {
            printf("✅ Nghiệm chính xác!\n");
        } else {
            printf("❌ Nghiệm không chính xác!\n");
        }
        
        printf("\n📊 Thông tin hiệu năng:\n");
        printf("   - Số luồng đã sử dụng: %d\n", num_threads);
        printf("   - Thời gian: %.6f giây\n", execution_time);
    } else {
        printf("❌ Giải thất bại!\n");
    }
    
    // Giải phóng bộ nhớ
    free_system(sys);
    
    return success ? 0 : 1;
}
#endif 