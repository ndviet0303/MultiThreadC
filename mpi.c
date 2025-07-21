/**
 * GAUSSIAN ELIMINATION - PHIÊN BẢN MPI
 * Giải hệ phương trình tuyến tính với distributed memory parallelism
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

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
    double max_error = 0.0;
    int error_count = 0;
    
    for (int i = 0; i < n; i++) {
        double sum = 0.0;
        for (int j = 0; j < n; j++) {
            sum += sys->A[i][j] * sys->x[j];
        }
        
        double error = fabs(sum - sys->b[i]);
        if (error > max_error) {
            max_error = error;
        }
        
        if (error > tolerance) {
            error_count++;
        }
    }
    
    // Adaptive tolerance cho ma trận lớn
    double adaptive_tolerance = (n > 2000) ? 1e-6 : tolerance;
    
    if (n > 2000 && max_error <= adaptive_tolerance) {
        return 1;  // Pass với adaptive tolerance
    }
    
    return (error_count == 0) ? 1 : 0;
}

/**
 * Thuật toán Gaussian Elimination sử dụng MPI
 * Phân phối hàng cho các processes
 */
int gaussian_elimination_mpi(LinearSystem *sys, int rank, int size) {
    int n = sys->n;
    double **A = sys->A;
    double *b = sys->b;
    double *x = sys->x;
    
    // Tính toán phân phối hàng
    int rows_per_proc = n / size;
    int extra_rows = n % size;
    int start_row = rank * rows_per_proc + (rank < extra_rows ? rank : extra_rows);
    int local_rows = rows_per_proc + (rank < extra_rows ? 1 : 0);
    int end_row = start_row + local_rows;
    
    if (rank == 0) {
        printf("Phân phối công việc:\n");
        for (int i = 0; i < size; i++) {
            int proc_start = i * rows_per_proc + (i < extra_rows ? i : extra_rows);
            int proc_rows = rows_per_proc + (i < extra_rows ? 1 : 0);
            printf("  Process %d: hàng %d - %d (%d hàng)\n", 
                   i, proc_start, proc_start + proc_rows - 1, proc_rows);
        }
        printf("\n");
    }
    
    // Buffer để lưu trữ hàng pivot
    double *pivot_row = malloc((n + 1) * sizeof(double));
    
    // Giai đoạn 1: Khử xuôi
    for (int k = 0; k < n - 1; k++) {
        int global_pivot_row = k;
        int pivot_owner = -1;
        
        // Xác định process nào sở hữu hàng k
        for (int i = 0; i < size; i++) {
            int proc_start = i * rows_per_proc + (i < extra_rows ? i : extra_rows);
            int proc_rows = rows_per_proc + (i < extra_rows ? 1 : 0);
            if (k >= proc_start && k < proc_start + proc_rows) {
                pivot_owner = i;
                break;
            }
        }
        
        // Tìm pivot lớn nhất
        double local_pivot_value = -1.0;
        int local_pivot_row = k;
        
        // Mỗi process tìm pivot trong phần của mình
        for (int i = start_row; i < end_row; i++) {
            if (i >= k && fabs(A[i][k]) > local_pivot_value) {
                local_pivot_value = fabs(A[i][k]);
                local_pivot_row = i;
            }
        }
        
        // Thu thập thông tin pivot từ tất cả processes
        struct {
            double value;
            int rank;
        } local_max, global_max;
        
        local_max.value = local_pivot_value;
        local_max.rank = rank;
        
        MPI_Allreduce(&local_max, &global_max, 1, MPI_DOUBLE_INT, MPI_MAXLOC, MPI_COMM_WORLD);
        
        // Process có pivot lớn nhất broadcast hàng pivot
        if (rank == global_max.rank) {
            // Copy hàng pivot vào buffer
            for (int j = 0; j < n; j++) {
                pivot_row[j] = A[local_pivot_row][j];
            }
            pivot_row[n] = b[local_pivot_row];  // Thêm phần tử từ vector b
            global_pivot_row = local_pivot_row;
        }
        
        // Broadcast thông tin pivot
        MPI_Bcast(&global_pivot_row, 1, MPI_INT, global_max.rank, MPI_COMM_WORLD);
        MPI_Bcast(pivot_row, n + 1, MPI_DOUBLE, global_max.rank, MPI_COMM_WORLD);
        
        // Kiểm tra tính khả nghịch
        if (fabs(pivot_row[k]) < 1e-12) {
            if (rank == 0) {
                printf("Lỗi: Ma trận không khả nghịch (pivot ≈ 0)\n");
            }
            free(pivot_row);
            return 0;
        }
        
        // Hoán đổi hàng thông minh: swap giữa processes
        if (global_pivot_row != k) {
            if (rank == pivot_owner) {
                // Process owns hàng k: gửi hàng k cho process có pivot
                if (k >= start_row && k < end_row) {
                    MPI_Send(A[k], n, MPI_DOUBLE, global_max.rank, k, MPI_COMM_WORLD);
                    MPI_Send(&b[k], 1, MPI_DOUBLE, global_max.rank, k + n, MPI_COMM_WORLD);
                }
            }
            
            if (rank == global_max.rank && rank != pivot_owner) {
                // Process có pivot: nhận hàng k và thay thế
                double *temp_row = malloc(n * sizeof(double));
                double temp_b;
                MPI_Recv(temp_row, n, MPI_DOUBLE, pivot_owner, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&temp_b, 1, MPI_DOUBLE, pivot_owner, k + n, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                // Swap trong local storage
                if (global_pivot_row >= start_row && global_pivot_row < end_row) {
                    for (int j = 0; j < n; j++) {
                        A[global_pivot_row][j] = temp_row[j];
                    }
                    b[global_pivot_row] = temp_b;
                }
                free(temp_row);
            }
            
            // Process owns hàng k: nhận pivot row
            if (rank == pivot_owner && k >= start_row && k < end_row) {
                for (int j = 0; j < n; j++) {
                    A[k][j] = pivot_row[j];
                }
                b[k] = pivot_row[n];
            }
        }
        
        // Thực hiện khử trong phần của mình
        for (int i = start_row; i < end_row; i++) {
            if (i > k) {
                double factor = A[i][k] / pivot_row[k];
                
                for (int j = k; j < n; j++) {
                    A[i][j] -= factor * pivot_row[j];
                }
                b[i] -= factor * pivot_row[n];
            }
        }
        
        // Đồng bộ hóa
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    // Thu thập ma trận về process 0 để thực hiện backward substitution
    if (rank == 0) {
        // Nhận dữ liệu từ các process khác
        for (int proc = 1; proc < size; proc++) {
            int proc_start = proc * rows_per_proc + (proc < extra_rows ? proc : extra_rows);
            int proc_rows = rows_per_proc + (proc < extra_rows ? 1 : 0);
            
            for (int i = 0; i < proc_rows; i++) {
                MPI_Recv(A[proc_start + i], n, MPI_DOUBLE, proc, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&b[proc_start + i], 1, MPI_DOUBLE, proc, i + n, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        
        // Thực hiện backward substitution
        for (int i = n - 1; i >= 0; i--) {
            x[i] = b[i];
            
            for (int j = i + 1; j < n; j++) {
                x[i] -= A[i][j] * x[j];
            }
            
            x[i] /= A[i][i];
        }
    } else {
        // Gửi dữ liệu về process 0
        for (int i = 0; i < local_rows; i++) {
            MPI_Send(A[start_row + i], n, MPI_DOUBLE, 0, i, MPI_COMM_WORLD);
            MPI_Send(&b[start_row + i], 1, MPI_DOUBLE, 0, i + n, MPI_COMM_WORLD);
        }
    }
    
    // Broadcast nghiệm về tất cả processes
    MPI_Bcast(x, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    free(pivot_row);
    return 1;
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
    int rank, size;
    int n = 100; // Kích thước mặc định
    
    // Khởi tạo MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) {
            if (rank == 0) {
                printf("Kích thước ma trận phải > 0\n");
            }
            MPI_Finalize();
            return 1;
        }
    }
    
    if (rank == 0) {
        printf("🧮 GAUSSIAN ELIMINATION - PHIÊN BẢN MPI\n");
        printf("Kích thước ma trận: %d x %d\n", n, n);
        printf("Số processes: %d\n\n", size);
    }
    
    // Tạo hệ phương trình (mỗi process tạo bản sao)
    LinearSystem *sys = create_system(n);
    
    // Chỉ process 0 tạo dữ liệu test
    if (rank == 0) {
        generate_test_system(sys);
        
        // Hiển thị ma trận nếu nhỏ
        if (n <= 10) {
            print_matrix(sys);
            print_vector(sys->b, n, "Vector b");
            printf("\n");
        }
    }
    
    // Broadcast ma trận và vector b từ process 0 đến tất cả
    for (int i = 0; i < n; i++) {
        MPI_Bcast(sys->A[i], n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    MPI_Bcast(sys->b, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    // Đo thời gian (sử dụng MPI timer)
    double start_time = MPI_Wtime();
    
    int success = gaussian_elimination_mpi(sys, rank, size);
    
    double end_time = MPI_Wtime();
    double elapsed_time = end_time - start_time;
    
    // Chỉ process 0 in kết quả
    if (rank == 0) {
        if (success) {
            printf("✅ Giải thành công!\n");
            printf("⏱️  Thời gian thực hiện: %.6f giây\n", elapsed_time);
            
            // Hiển thị nghiệm nếu ma trận nhỏ
            if (n <= 10) {
                print_vector(sys->x, n, "Nghiệm x");
            }
            
            // Kiểm tra tính đúng đắn của nghiệm
            if (verify_solution(sys)) {
                printf("✅ Nghiệm chính xác!\n");
            } else {
                printf("❌ Nghiệm không chính xác!\n");
            }
            
            // Thông tin về hiệu năng
            printf("\n📊 Thông tin hiệu năng:\n");
            printf("   - Số processes: %d\n", size);
            printf("   - Thời gian: %.6f giây\n", elapsed_time);
            
        } else {
            printf("❌ Không thể giải hệ phương trình!\n");
        }
    }
    
    // Dọn dẹp bộ nhớ
    free_system(sys);
    
    // Kết thúc MPI
    MPI_Finalize();
    
    return success ? 0 : 1;
} 