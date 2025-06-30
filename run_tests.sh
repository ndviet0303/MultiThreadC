#!/bin/bash

# Script chạy test toàn diện cho Gaussian Elimination
# Hỗ trợ Sequential, OpenMP, Pthread, MPI

set -e  # Exit on error

echo "🚀 GAUSSIAN ELIMINATION - COMPREHENSIVE TEST SUITE"
echo "=================================================="

# Màu sắc cho output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Hàm in màu
print_color() {
    echo -e "${1}${2}${NC}"
}

# Kiểm tra dependencies
check_dependencies() {
    print_color $YELLOW "📋 Kiểm tra dependencies..."
    
    if ! command -v gcc &> /dev/null; then
        print_color $RED "❌ gcc không được tìm thấy!"
        exit 1
    fi
    
    if ! command -v make &> /dev/null; then
        print_color $RED "❌ make không được tìm thấy!"
        exit 1
    fi
    
    if command -v mpicc &> /dev/null && command -v mpirun &> /dev/null; then
        MPI_AVAILABLE=1
        print_color $GREEN "✅ MPI available"
    else
        MPI_AVAILABLE=0
        print_color $YELLOW "⚠️  MPI không có sẵn, bỏ qua test MPI"
    fi
    
    print_color $GREEN "✅ Dependencies OK"
}

# Build tất cả
build_all() {
    print_color $YELLOW "🔨 Building tất cả phiên bản..."
    
    if make clean && make all; then
        print_color $GREEN "✅ Build thành công!"
    else
        print_color $RED "❌ Build thất bại!"
        exit 1
    fi
}

# Test cơ bản với ma trận nhỏ
test_basic() {
    print_color $BLUE "\n🧪 TEST CƠ BẢN (ma trận 10x10)"
    echo "========================================"
    
    # Test Sequential
    print_color $YELLOW "Testing Sequential..."
    if ./build/sequential 10; then
        print_color $GREEN "✅ Sequential: PASS"
    else
        print_color $RED "❌ Sequential: FAIL"
    fi
    
    echo ""
    
    # Test OpenMP
    print_color $YELLOW "Testing OpenMP..."
    if ./build/openmp_version 10 4; then
        print_color $GREEN "✅ OpenMP: PASS"
    else
        print_color $RED "❌ OpenMP: FAIL"
    fi
    
    echo ""
    
    # Test Pthread
    print_color $YELLOW "Testing Pthread..."
    if ./build/pthread_version 10 4; then
        print_color $GREEN "✅ Pthread: PASS"
    else
        print_color $RED "❌ Pthread: FAIL"
    fi
    
    echo ""
    
    # Test MPI nếu có
    if [ $MPI_AVAILABLE -eq 1 ]; then
        print_color $YELLOW "Testing MPI..."
        if mpirun -np 4 ./build/mpi_version 10; then
            print_color $GREEN "✅ MPI: PASS"
        else
            print_color $RED "❌ MPI: FAIL"
        fi
    fi
}

# Test hiệu năng với các kích thước khác nhau
test_performance() {
    print_color $BLUE "\n⚡ TEST HIỆU NĂNG"
    echo "========================================"
    
    # Các kích thước test
    sizes=(100 200 500)
    
    for size in "${sizes[@]}"; do
        print_color $YELLOW "Testing với ma trận ${size}x${size}..."
        
        echo "📊 Performance comparison for n=$size"
        echo "----------------------------------------"
        
        # Sequential baseline
        print_color $YELLOW "Sequential (baseline)..."
        seq_time=$(./build/sequential $size | grep "Thời gian thực hiện" | awk '{print $4}')
        echo "Sequential: ${seq_time} giây"
        
        # Test OpenMP với số luồng khác nhau
        for threads in 2 4 8; do
            print_color $YELLOW "OpenMP với $threads luồng..."
            omp_time=$(./build/openmp_version $size $threads | grep "Thời gian thực hiện" | awk '{print $4}')
            if [ ! -z "$seq_time" ] && [ ! -z "$omp_time" ]; then
                speedup=$(echo "scale=2; $seq_time / $omp_time" | bc -l 2>/dev/null || echo "N/A")
                echo "OpenMP ($threads threads): ${omp_time} giây (Speedup: ${speedup}x)"
            fi
        done
        
        # Test Pthread với số luồng khác nhau
        for threads in 2 4 8; do
            print_color $YELLOW "Pthread với $threads luồng..."
            pthread_time=$(./build/pthread_version $size $threads | grep "Thời gian thực hiện" | awk '{print $4}')
            if [ ! -z "$seq_time" ] && [ ! -z "$pthread_time" ]; then
                speedup=$(echo "scale=2; $seq_time / $pthread_time" | bc -l 2>/dev/null || echo "N/A")
                echo "Pthread ($threads threads): ${pthread_time} giây (Speedup: ${speedup}x)"
            fi
        done
        
        echo ""
    done
}

# Test tính đúng đắn với ma trận lớn
test_correctness() {
    print_color $BLUE "\n🎯 TEST TÍNH ĐÚNG ĐẮN"
    echo "========================================"
    
    size=100
    print_color $YELLOW "Kiểm tra tính đúng đắn với ma trận ${size}x${size}..."
    
    # Tạo file tạm để lưu nghiệm
    seq_result=$(mktemp)
    omp_result=$(mktemp)
    pthread_result=$(mktemp)
    
    # Chạy các phiên bản và lưu nghiệm
    ./build/sequential $size > $seq_result 2>&1
    ./build/openmp_version $size 4 > $omp_result 2>&1
    ./build/pthread_version $size 4 > $pthread_result 2>&1
    
    # Kiểm tra xem tất cả có thành công không
    if grep -q "Nghiệm chính xác" $seq_result && \
       grep -q "Nghiệm chính xác" $omp_result && \
       grep -q "Nghiệm chính xác" $pthread_result; then
        print_color $GREEN "✅ Tất cả phiên bản cho nghiệm chính xác!"
    else
        print_color $RED "❌ Có phiên bản cho nghiệm không chính xác!"
    fi
    
    # Dọn dẹp
    rm -f $seq_result $omp_result $pthread_result
}

# Test stress với ma trận lớn
test_stress() {
    print_color $BLUE "\n💪 STRESS TEST"
    echo "========================================"
    
    size=1000
    print_color $YELLOW "Stress test với ma trận ${size}x${size}..."
    
    # Test Sequential
    print_color $YELLOW "Sequential stress test..."
    if timeout 60 ./build/sequential $size > /dev/null; then
        print_color $GREEN "✅ Sequential stress test: PASS"
    else
        print_color $RED "❌ Sequential stress test: FAIL (timeout hoặc lỗi)"
    fi
    
    # Test OpenMP
    print_color $YELLOW "OpenMP stress test..."
    if timeout 30 ./build/openmp_version $size 8 > /dev/null; then
        print_color $GREEN "✅ OpenMP stress test: PASS"
    else
        print_color $RED "❌ OpenMP stress test: FAIL (timeout hoặc lỗi)"
    fi
    
    # Test Pthread
    print_color $YELLOW "Pthread stress test..."
    if timeout 30 ./build/pthread_version $size 8 > /dev/null; then
        print_color $GREEN "✅ Pthread stress test: PASS"
    else
        print_color $RED "❌ Pthread stress test: FAIL (timeout hoặc lỗi)"
    fi
}

# Chạy comprehensive performance test
test_comprehensive_performance() {
    print_color $BLUE "\n📈 COMPREHENSIVE PERFORMANCE TEST"
    echo "========================================"
    
    if [ -f "./build/performance_test" ]; then
        print_color $YELLOW "Chạy comprehensive performance test..."
        ./build/performance_test 300
        
        if [ -f "performance_results.csv" ]; then
            print_color $GREEN "✅ Kết quả được lưu trong performance_results.csv"
        fi
    else
        print_color $RED "❌ Performance test binary không tồn tại!"
    fi
}

# Main function
main() {
    # Parse arguments
    RUN_BASIC=1
    RUN_PERFORMANCE=1
    RUN_CORRECTNESS=1
    RUN_STRESS=0
    RUN_COMPREHENSIVE=1
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --basic-only)
                RUN_PERFORMANCE=0
                RUN_CORRECTNESS=0
                RUN_COMPREHENSIVE=0
                shift
                ;;
            --performance-only)
                RUN_BASIC=0
                RUN_CORRECTNESS=0
                RUN_COMPREHENSIVE=0
                shift
                ;;
            --with-stress)
                RUN_STRESS=1
                shift
                ;;
            --help)
                echo "Usage: $0 [options]"
                echo "Options:"
                echo "  --basic-only      Chỉ chạy test cơ bản"
                echo "  --performance-only Chỉ chạy test hiệu năng"
                echo "  --with-stress     Bao gồm stress test"
                echo "  --help            Hiển thị trợ giúp"
                exit 0
                ;;
            *)
                echo "Unknown option: $1"
                echo "Use --help for usage information"
                exit 1
                ;;
        esac
    done
    
    # Chạy các test
    check_dependencies
    build_all
    
    if [ $RUN_BASIC -eq 1 ]; then
        test_basic
    fi
    
    if [ $RUN_CORRECTNESS -eq 1 ]; then
        test_correctness
    fi
    
    if [ $RUN_PERFORMANCE -eq 1 ]; then
        test_performance
    fi
    
    if [ $RUN_STRESS -eq 1 ]; then
        test_stress
    fi
    
    if [ $RUN_COMPREHENSIVE -eq 1 ]; then
        test_comprehensive_performance
    fi
    
    print_color $GREEN "\n🎉 TẤT CẢ TEST ĐÃ HOÀN THÀNH!"
    echo ""
    echo "📋 Tóm tắt:"
    echo "   - Sequential: Gaussian Elimination tuần tự"
    echo "   - OpenMP: Song song hóa với shared memory"
    echo "   - Pthread: Song song hóa với manual thread management"
    echo "   - MPI: Song song hóa với distributed memory"
    echo ""
    echo "📊 Kết quả chi tiết có thể xem trong performance_results.csv"
}

# Chạy script
main "$@" 