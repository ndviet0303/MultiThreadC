#include "utils.h"
#include <omp.h>
#include <pthread.h>

// Khai báo các hàm từ các file khác
extern int gaussian_elimination_sequential(LinearSystem *sys);
extern int gaussian_elimination_openmp(LinearSystem *sys, int num_threads);
extern int gaussian_elimination_pthread(LinearSystem *sys, int num_threads);

/**
 * Cấu trúc lưu trữ kết quả test
 */
typedef struct {
    char method[20];
    int n;
    int threads;
    double time;
    int success;
    double speedup;
    double efficiency;
} TestResult;

/**
 * Chạy test cho một phương pháp cụ thể
 */
double run_test(LinearSystem *original_sys, const char *method, int threads) {
    // Tạo bản sao của hệ phương trình
    LinearSystem *sys = create_system(original_sys->n);
    copy_system(original_sys, sys);
    
    struct timespec start, end;
    int success = 0;
    
    START_TIMER(start);
    
    if (strcmp(method, "sequential") == 0) {
        success = gaussian_elimination_sequential(sys);
    } else if (strcmp(method, "openmp") == 0) {
        success = gaussian_elimination_openmp(sys, threads);
    } else if (strcmp(method, "pthread") == 0) {
        success = gaussian_elimination_pthread(sys, threads);
    }
    
    END_TIMER(end);
    
    double elapsed_time = get_time_diff(start, end);
    
    // Kiểm tra tính đúng đắn
    if (success && !verify_solution(sys)) {
        printf("⚠️  Cảnh báo: Nghiệm %s không chính xác!\n", method);
        success = 0;
    }
    
    free_system(sys);
    return success ? elapsed_time : -1.0;
}

/**
 * In kết quả dưới dạng bảng
 */
void print_results_table(TestResult *results, int count, double sequential_time) {
    printf("\n📊 KẾT QUẢ KHẢO SÁT HIỆU NĂNG\n");
    printf("================================================================================\n");
    printf("%-12s %-8s %-8s %-12s %-10s %-12s\n", 
           "Phương pháp", "Kích thước", "Luồng", "Thời gian(s)", "Speedup", "Efficiency");
    printf("================================================================================\n");
    
    for (int i = 0; i < count; i++) {
        if (results[i].success) {
            printf("%-12s %-8d %-8d %-12.6f %-10.2f %-12.2f\n",
                   results[i].method, results[i].n, results[i].threads,
                   results[i].time, results[i].speedup, results[i].efficiency);
        } else {
            printf("%-12s %-8d %-8d %-12s %-10s %-12s\n",
                   results[i].method, results[i].n, results[i].threads,
                   "FAILED", "-", "-");
        }
    }
    printf("================================================================================\n");
}

/**
 * Xuất kết quả ra file CSV
 */
void export_to_csv(TestResult *results, int count, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("⚠️  Không thể tạo file %s\n", filename);
        return;
    }
    
    fprintf(file, "Method,Size,Threads,Time,Speedup,Efficiency\n");
    for (int i = 0; i < count; i++) {
        if (results[i].success) {
            fprintf(file, "%s,%d,%d,%.6f,%.2f,%.2f\n",
                    results[i].method, results[i].n, results[i].threads,
                    results[i].time, results[i].speedup, results[i].efficiency);
        }
    }
    
    fclose(file);
    printf("✅ Đã xuất kết quả ra file %s\n", filename);
}

/**
 * Chương trình chính
 */
int main(int argc, char *argv[]) {
    // Cấu hình test mặc định
    int test_sizes[] = {100, 200, 500, 1000};
    int thread_counts[] = {3, 5, 7, 9, 11};
    int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    int num_thread_counts = sizeof(thread_counts) / sizeof(thread_counts[0]);
    
    // Cho phép người dùng chỉ định kích thước test
    if (argc > 1) {
        int custom_size = atoi(argv[1]);
        if (custom_size > 0) {
            test_sizes[0] = custom_size;
            num_sizes = 1;
        }
    }
    
    printf("🚀 BẮT ĐẦU KHẢO SÁT HIỆU NĂNG - GAUSSIAN ELIMINATION\n");
    printf("==================================================\n");
    printf("Số processors có sẵn: %d\n", omp_get_num_procs());
    printf("Kích thước test: ");
    for (int i = 0; i < num_sizes; i++) {
        printf("%d ", test_sizes[i]);
    }
    printf("\nSố luồng test: ");
    for (int i = 0; i < num_thread_counts; i++) {
        printf("%d ", thread_counts[i]);
    }
    printf("\n\n");
    
    // Mảng lưu trữ kết quả
    int max_results = num_sizes * (1 + 2 * num_thread_counts); // Sequential + OpenMP + Pthread
    TestResult *results = (TestResult*)malloc(max_results * sizeof(TestResult));
    int result_count = 0;
    
    // Chạy test cho từng kích thước
    for (int size_idx = 0; size_idx < num_sizes; size_idx++) {
        int n = test_sizes[size_idx];
        printf("📏 Testing với kích thước ma trận: %d x %d\n", n, n);
        
        // Tạo hệ phương trình test
        LinearSystem *original_sys = create_system(n);
        generate_test_system(original_sys);
        
        // Test phiên bản tuần tự (baseline)
        printf("   🔸 Sequential... ");
        fflush(stdout);
        double sequential_time = run_test(original_sys, "sequential", 1);
        
        if (sequential_time > 0) {
            printf("✅ %.6f giây\n", sequential_time);
            strcpy(results[result_count].method, "Sequential");
            results[result_count].n = n;
            results[result_count].threads = 1;
            results[result_count].time = sequential_time;
            results[result_count].speedup = 1.0;
            results[result_count].efficiency = 1.0;
            results[result_count].success = 1;
            result_count++;
        } else {
            printf("❌ FAILED\n");
            continue; // Bỏ qua kích thước này nếu sequential fail
        }
        
        // Test các phiên bản song song
        for (int thread_idx = 0; thread_idx < num_thread_counts; thread_idx++) {
            int threads = thread_counts[thread_idx];
            
            // Test OpenMP
            printf("   🔸 OpenMP (%d threads)... ", threads);
            fflush(stdout);
            double openmp_time = run_test(original_sys, "openmp", threads);
            
            if (openmp_time > 0) {
                double speedup = sequential_time / openmp_time;
                double efficiency = speedup / threads;
                printf("✅ %.6f giây (Speedup: %.2fx)\n", openmp_time, speedup);
                
                strcpy(results[result_count].method, "OpenMP");
                results[result_count].n = n;
                results[result_count].threads = threads;
                results[result_count].time = openmp_time;
                results[result_count].speedup = speedup;
                results[result_count].efficiency = efficiency;
                results[result_count].success = 1;
                result_count++;
            } else {
                printf("❌ FAILED\n");
            }
            
            // Test Pthread
            printf("   🔸 Pthread (%d threads)... ", threads);
            fflush(stdout);
            double pthread_time = run_test(original_sys, "pthread", threads);
            
            if (pthread_time > 0) {
                double speedup = sequential_time / pthread_time;
                double efficiency = speedup / threads;
                printf("✅ %.6f giây (Speedup: %.2fx)\n", pthread_time, speedup);
                
                strcpy(results[result_count].method, "Pthread");
                results[result_count].n = n;
                results[result_count].threads = threads;
                results[result_count].time = pthread_time;
                results[result_count].speedup = speedup;
                results[result_count].efficiency = efficiency;
                results[result_count].success = 1;
                result_count++;
            } else {
                printf("❌ FAILED\n");
            }
        }
        
        free_system(original_sys);
        printf("\n");
    }
    
    // In kết quả tổng hợp
    if (result_count > 0) {
        print_results_table(results, result_count, 0);
        export_to_csv(results, result_count, "performance_results.csv");
        
        // Phân tích kết quả
        printf("\n📈 PHÂN TÍCH KẾT QUẢ:\n");
        printf("=====================================\n");
        
        // Tìm speedup tốt nhất
        double best_speedup = 0.0;
        TestResult best_result;
        for (int i = 0; i < result_count; i++) {
            if (results[i].success && results[i].speedup > best_speedup) {
                best_speedup = results[i].speedup;
                best_result = results[i];
            }
        }
        
        if (best_speedup > 1.0) {
            printf("🏆 Speedup tốt nhất: %.2fx (%s với %d luồng)\n", 
                   best_result.speedup, best_result.method, best_result.threads);
            printf("🎯 Efficiency tương ứng: %.2f%%\n", best_result.efficiency * 100);
        }
        
        printf("\n💡 GỢI Ý CẢI TIẾN:\n");
        printf("- Với ma trận nhỏ (n < 500): Sử dụng phiên bản tuần tự\n");
        printf("- Với ma trận lớn (n ≥ 500): Sử dụng OpenMP với số luồng = số core\n");
        printf("- Pthread phù hợp khi cần kiểm soát chi tiết về luồng\n");
        printf("- MPI phù hợp cho tính toán phân tán trên nhiều máy\n");
    }
    
    free(results);
    
    printf("\n🎉 Hoàn thành khảo sát hiệu năng!\n");
    return 0;
} 