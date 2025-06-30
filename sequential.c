#include "utils.h"



/**
 * Chương trình test phiên bản tuần tự
 */
#ifndef LIB_MODE
int main(int argc, char *argv[]) {
    int n = 100; // Kích thước mặc định
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) {
            printf("Kích thước ma trận phải > 0\n");
            return 1;
        }
    }
    
    printf("=== PHIÊN BẢN TUẦN TỰ - GAUSSIAN ELIMINATION ===\n");
    printf("Kích thước ma trận: %d x %d\n\n", n, n);
    
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
    
    int success = gaussian_elimination_sequential(sys);
    
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
    } else {
        printf("❌ Không thể giải hệ phương trình!\n");
    }
    
    // Dọn dẹp bộ nhớ
    free_system(sys);
    
    return 0;
}
#endif // LIB_MODE 