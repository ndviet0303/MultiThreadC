#include <stdio.h>
#include "utils.h"

/**
 * Chương trình demo đơn giản cho Gaussian Elimination
 */
int main(void) {
    printf("🧮 GAUSSIAN ELIMINATION - DEMO\n");
    printf("==============================\n\n");
    
    printf("Dự án này triển khai thuật toán Gaussian Elimination với các mô hình song song:\n");
    printf("1. Sequential - Phiên bản tuần tự\n");
    printf("2. OpenMP - Song song hóa với shared memory\n");
    printf("3. Pthreads - Song song hóa với manual threads\n");
    printf("4. MPI - Song song hóa với distributed memory\n\n");
    
    printf("📁 Cấu trúc file:\n");
    printf("   - utils.h/utils.c: Hàm tiện ích chung\n");
    printf("   - sequential.c: Phiên bản tuần tự\n");
    printf("   - openmp_version.c: Phiên bản OpenMP\n");
    printf("   - pthread_version.c: Phiên bản Pthreads\n");
    printf("   - mpi_version.c: Phiên bản MPI\n");
    printf("   - performance_test.c: Test hiệu năng\n");
    printf("   - Makefile: Build script\n");
    printf("   - run_tests.sh: Test script\n");
    printf("   - README.md: Hướng dẫn chi tiết\n\n");
    
    printf("🚀 Cách sử dụng:\n");
    printf("   1. Build: make all\n");
    printf("   2. Test: make test\n");
    printf("   3. Demo: ./run_tests.sh\n");
    printf("   4. Help: make help\n\n");
    
    printf("📖 Xem README.md để biết hướng dẫn chi tiết!\n");
    
    // Demo nhỏ với ma trận 3x3
    printf("\n🔬 DEMO NHỎ - Ma trận 3x3:\n");
    printf("===========================\n");
    
    LinearSystem *demo_sys = create_system(3);
    
    // Tạo hệ phương trình đơn giản
    // 2x + y + z = 8
    // x + 3y + z = 11  
    // x + y + 4z = 16
    demo_sys->A[0][0] = 2; demo_sys->A[0][1] = 1; demo_sys->A[0][2] = 1;
    demo_sys->A[1][0] = 1; demo_sys->A[1][1] = 3; demo_sys->A[1][2] = 1;
    demo_sys->A[2][0] = 1; demo_sys->A[2][1] = 1; demo_sys->A[2][2] = 4;
    
    demo_sys->b[0] = 8;
    demo_sys->b[1] = 11;
    demo_sys->b[2] = 16;
    
    printf("Hệ phương trình:\n");
    printf("2x + y + z = 8\n");
    printf("x + 3y + z = 11\n");
    printf("x + y + 4z = 16\n\n");
    
    print_matrix(demo_sys->A, 3);
    printf("Vector b: ");
    print_vector(demo_sys->b, 3);
    
    // Giải bằng phương pháp tuần tự
    if (gaussian_elimination_sequential(demo_sys)) {
        printf("\n✅ Nghiệm tìm được: ");
        print_vector(demo_sys->x, 3);
        
        // Verify
        if (verify_solution(demo_sys)) {
            printf("✅ Nghiệm chính xác!\n");
        }
    }
    
    free_system(demo_sys);
    
    printf("\n🎯 Để test với ma trận lớn hơn, sử dụng:\n");
    printf("   ./build/sequential 1000\n");
    printf("   ./build/openmp_version 1000 8\n");
    printf("   ./build/performance_test 500\n");
    
    return 0;
}