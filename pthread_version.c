#include "utils.h"
#include <pthread.h>

// Cấu trúc dữ liệu cho các luồng
typedef struct {
    LinearSystem *sys;
    int start_row;
    int end_row;
    int k;              // Bước khử hiện tại
    int num_threads;
    pthread_barrier_t *barrier;
} ThreadData;

// Biến toàn cục để đồng bộ
static pthread_barrier_t barrier;
static int pivot_row;
static double pivot_value;
static pthread_mutex_t pivot_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Hàm tìm pivot lớn nhất trong cột k (song song)
 */
void find_pivot_parallel(LinearSystem *sys, int k, int start_row, int end_row) {
    int local_max_row = k;
    double local_max_val = (start_row <= k) ? fabs(sys->A[k][k]) : 0.0;
    
    // Tìm pivot trong phạm vi được gán
    for (int i = (start_row > k) ? start_row : k + 1; i < end_row && i < sys->n; i++) {
        if (fabs(sys->A[i][k]) > local_max_val) {
            local_max_val = fabs(sys->A[i][k]);
            local_max_row = i;
        }
    }
    
    // Cập nhật pivot toàn cục (cần đồng bộ)
    pthread_mutex_lock(&pivot_mutex);
    if (local_max_val > pivot_value) {
        pivot_value = local_max_val;
        pivot_row = local_max_row;
    }
    pthread_mutex_unlock(&pivot_mutex);
}

/**
 * Hàm thực hiện bởi mỗi luồng
 */
void* thread_gaussian_elimination(void* arg) {
    ThreadData *data = (ThreadData*)arg;
    LinearSystem *sys = data->sys;
    int n = sys->n;
    
    // Thực hiện khử xuôi
    for (int k = 0; k < n - 1; k++) {
        // Reset pivot cho bước mới
        if (data->start_row == 0) {  // Luồng đầu tiên
            pivot_row = k;
            pivot_value = fabs(sys->A[k][k]);
        }
        
        // Đồng bộ trước khi tìm pivot
        pthread_barrier_wait(data->barrier);
        
        // Tìm pivot trong phạm vi được gán
        find_pivot_parallel(sys, k, data->start_row, data->end_row);
        
        // Đồng bộ sau khi tìm pivot
        pthread_barrier_wait(data->barrier);
        
        // Kiểm tra ma trận có khả nghịch không (chỉ luồng đầu tiên)
        if (data->start_row == 0) {
            if (pivot_value < 1e-12) {
                printf("Lỗi: Ma trận không khả nghịch (pivot = 0)\n");
                pthread_exit(NULL);
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
        }
        
        // Đồng bộ sau khi hoán đổi
        pthread_barrier_wait(data->barrier);
        
        // Thực hiện khử trong phạm vi được gán
        for (int i = data->start_row; i < data->end_row && i < n; i++) {
            if (i > k) {  // Chỉ khử các hàng dưới pivot
                double factor = sys->A[i][k] / sys->A[k][k];
                
                // Cập nhật hàng i
                for (int j = k; j < n; j++) {
                    sys->A[i][j] -= factor * sys->A[k][j];
                }
                sys->b[i] -= factor * sys->b[k];
            }
        }
        
        // Đồng bộ sau khi hoàn thành bước k
        pthread_barrier_wait(data->barrier);
    }
    
    return NULL;
}

/**
 * Thuật toán Gaussian Elimination sử dụng Pthreads
 */
int gaussian_elimination_pthread(LinearSystem *sys, int num_threads) {
    int n = sys->n;
    
    // Khởi tạo barrier
    pthread_barrier_init(&barrier, NULL, num_threads);
    
    // Tạo các luồng
    pthread_t *threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    ThreadData *thread_data = (ThreadData*)malloc(num_threads * sizeof(ThreadData));
    
    // Chia công việc cho các luồng
    int rows_per_thread = n / num_threads;
    int remaining_rows = n % num_threads;
    
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].sys = sys;
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i + 1) * rows_per_thread;
        thread_data[i].num_threads = num_threads;
        thread_data[i].barrier = &barrier;
        
        // Phân bổ các hàng còn lại cho luồng cuối
        if (i == num_threads - 1) {
            thread_data[i].end_row += remaining_rows;
        }
    }
    
    // Tạo và chạy các luồng
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, thread_gaussian_elimination, &thread_data[i]) != 0) {
            printf("Lỗi: Không thể tạo luồng %d\n", i);
            return 0;
        }
    }
    
    // Chờ các luồng hoàn thành
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Kiểm tra phần tử cuối cùng trên đường chéo
    if (fabs(sys->A[n-1][n-1]) < 1e-12) {
        printf("Lỗi: Ma trận không khả nghịch\n");
        pthread_barrier_destroy(&barrier);
        free(threads);
        free(thread_data);
        return 0;
    }
    
    // Giai đoạn 2: Thế ngược (tuần tự vì khó song song hóa)
    for (int i = n - 1; i >= 0; i--) {
        sys->x[i] = sys->b[i];
        
        for (int j = i + 1; j < n; j++) {
            sys->x[i] -= sys->A[i][j] * sys->x[j];
        }
        
        sys->x[i] /= sys->A[i][i];
    }
    
    // Dọn dẹp
    pthread_barrier_destroy(&barrier);
    free(threads);
    free(thread_data);
    
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
    
    // Đo thời gian giải
    struct timespec start, end;
    START_TIMER(start);
    
    int success = gaussian_elimination_pthread(sys, num_threads);
    
    END_TIMER(end);
    double elapsed_time = get_time_diff(start, end);
    
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