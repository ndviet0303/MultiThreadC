#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

// Cấu trúc lưu trữ hệ phương trình
typedef struct {
    double **A;     // Ma trận hệ số
    double *b;      // Vector hằng số
    double *x;      // Vector nghiệm
    int n;          // Kích thước ma trận
} LinearSystem;

// Các hàm tiện ích chung
LinearSystem* create_system(int n);
void free_system(LinearSystem *sys);
void generate_test_system(LinearSystem *sys);
void print_matrix(double **A, int n);
void print_vector(double *v, int n);
int verify_solution(LinearSystem *sys);
double get_time_diff(struct timespec start, struct timespec end);

// Các hàm giải hệ phương trình
int gaussian_elimination_sequential(LinearSystem *sys);
int gaussian_elimination_openmp(LinearSystem *sys, int num_threads);
int gaussian_elimination_pthread(LinearSystem *sys, int num_threads);

// Hàm copy hệ phương trình để test
void copy_system(LinearSystem *src, LinearSystem *dest);

// Macro để đo thời gian
#define START_TIMER(start) clock_gettime(CLOCK_MONOTONIC, &start)
#define END_TIMER(end) clock_gettime(CLOCK_MONOTONIC, &end)

#endif // UTILS_H 