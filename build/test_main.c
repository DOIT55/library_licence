/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_main.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 14:05:41 by HaJuYoung(juha)   #+#    #+#             */
/*   Updated: 2025/08/29 16:02:34 by HaJuYoung(juha)  ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "liblicense.h"
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// CLOCK_MONOTONIC이 정의되지 않은 경우 대체
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

// 나노초를 마이크로초로 변환하는 매크로
#define NS_TO_US(ns) ((ns) / 1000.0)
#define NS_TO_MS(ns) ((ns) / 1000000.0)

// 통계 정보 구조체
typedef struct {
    char function_name[64];
    long* measurements;
    int count;
    double mean;
    double variance;
    double std_dev;
    long min_time;
    long max_time;
    double median;
} function_stats_t;

// 두 timespec 구조체의 차이를 나노초로 계산
static long timespec_diff_ns(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000000000L + (end->tv_nsec - start->tv_nsec);
}

// long 배열의 중간값 계산을 위한 비교 함수
static int compare_long(const void *a, const void *b) {
    long la = *(const long*)a;
    long lb = *(const long*)b;
    return (la > lb) - (la < lb);
}

// 통계 계산 함수
static void calculate_statistics(function_stats_t* stats) {
    if (stats->count == 0) return;
    
    // 평균 계산
    long sum = 0;
    for (int i = 0; i < stats->count; i++) {
        sum += stats->measurements[i];
    }
    stats->mean = (double)sum / stats->count;
    
    // 최소값, 최대값 찾기
    stats->min_time = stats->measurements[0];
    stats->max_time = stats->measurements[0];
    for (int i = 1; i < stats->count; i++) {
        if (stats->measurements[i] < stats->min_time) {
            stats->min_time = stats->measurements[i];
        }
        if (stats->measurements[i] > stats->max_time) {
            stats->max_time = stats->measurements[i];
        }
    }
    
    // 분산 계산
    double variance_sum = 0;
    for (int i = 0; i < stats->count; i++) {
        double diff = stats->measurements[i] - stats->mean;
        variance_sum += diff * diff;
    }
    stats->variance = variance_sum / stats->count;
    stats->std_dev = sqrt(stats->variance);
    
    // 중간값 계산 (정렬된 복사본 사용)
    long* sorted = malloc(stats->count * sizeof(long));
    memcpy(sorted, stats->measurements, stats->count * sizeof(long));
    qsort(sorted, stats->count, sizeof(long), compare_long);
    
    if (stats->count % 2 == 0) {
        stats->median = (sorted[stats->count/2 - 1] + sorted[stats->count/2]) / 2.0;
    } else {
        stats->median = sorted[stats->count/2];
    }
    
    free(sorted);
}

// 성능 측정 결과 출력
static void print_performance(const char* func_name, long ns) {
    printf(COLOR_CYAN "%-30s: " COLOR_RESET, func_name);
    if (ns < 1000) {
        printf("%ld ns\n", ns);
    } else if (ns < 1000000) {
        printf("%.2f μs\n", NS_TO_US(ns));
    } else {
        printf("%.2f ms\n", NS_TO_MS(ns));
    }
}

// 통계 정보 출력 함수
static void print_function_statistics(function_stats_t* stats) {
    printf("\n" COLOR_BOLD COLOR_YELLOW "=== %s Statistics ===" COLOR_RESET "\n", stats->function_name);
    printf("┌─────────────────────┬──────────────────┬──────────────────┐\n");
    printf("│ %-19s │ %-16s │ %-16s │\n", "Metric", "Nanoseconds", "Microseconds");
    printf("├─────────────────────┼──────────────────┼──────────────────┤\n");
    printf("│ %-19s │ %13.2f ns │ %13.2f μs │\n", "Mean", stats->mean, NS_TO_US(stats->mean));
    printf("│ %-19s │ %13.2f ns │ %13.2f μs │\n", "Median", stats->median, NS_TO_US(stats->median));
    printf("│ %-19s │ %13ld ns │ %13.2f μs │\n", "Min", stats->min_time, NS_TO_US(stats->min_time));
    printf("│ %-19s │ %13ld ns │ %13.2f μs │\n", "Max", stats->max_time, NS_TO_US(stats->max_time));
    printf("│ %-19s │ %13.2f ns │ %13.2f μs │\n", "Std Deviation", stats->std_dev, NS_TO_US(stats->std_dev));
    printf("│ %-19s │ %13.2f ns² │ %13.2f μs² │\n", "Variance", stats->variance, stats->variance / 1000000.0);
    printf("└─────────────────────┴──────────────────┴──────────────────┘\n");
    
    printf("Sample Count: %d\n", stats->count);
    printf("CV (Coefficient of Variation): %.2f%%\n", (stats->std_dev / stats->mean) * 100);
    
    // 95% 신뢰구간 (정규분포 가정)
    double margin = 1.96 * (stats->std_dev / sqrt(stats->count));
    printf("95%% Confidence Interval: [%.2f, %.2f] μs\n", 
           NS_TO_US(stats->mean - margin), NS_TO_US(stats->mean + margin));
}

// 전체 라이센스 검증 프로세스 함수 (앞으로 이동)
static int full_license_check() {
    License_error_code code;
    int is_valid;
    
    // 1. 라이센스 파일 로드
    code = load_license_file();
    if (code != License_result_success) {
        return 0; // 실패
    }
    
    // 2. 만료 시간 확인 (void 캐스팅으로 unused 경고 제거)
    (void)get_license_expire_time();
    
    // 3. 생성 시간 확인 (void 캐스팅으로 unused 경고 제거)
    (void)get_license_create_date();
    
    // 4. 유효성 검사
    is_valid = is_license_valid_period();
    
    // 5. 에러 코드 문자열 확인
    (void)license_error_code_to_string(License_result_success);
    
    return is_valid; // 성공 여부 반환
}

// 개별 함수 성능 통계 측정
static function_stats_t* measure_function_performance(const char* func_name, int iterations) {
    function_stats_t* stats = malloc(sizeof(function_stats_t));
    if (!stats) return NULL;
    
    strncpy(stats->function_name, func_name, sizeof(stats->function_name) - 1);
    stats->function_name[sizeof(stats->function_name) - 1] = '\0';
    stats->count = iterations;
    stats->measurements = malloc(iterations * sizeof(long));
    if (!stats->measurements) {
        free(stats);
        return NULL;
    }
    
    struct timespec start, end;
    
    printf("Measuring %s performance (%d iterations)...\n", func_name, iterations);
    
    for (int i = 0; i < iterations; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        // 함수별 측정
        if (strcmp(func_name, "load_license_file") == 0) {
            load_license_file();
        } else if (strcmp(func_name, "get_license_expire_time") == 0) {
            (void)get_license_expire_time();
        } else if (strcmp(func_name, "get_license_create_date") == 0) {
            (void)get_license_create_date();
        } else if (strcmp(func_name, "is_license_valid_period") == 0) {
            (void)is_license_valid_period();
        } else if (strcmp(func_name, "license_error_code_to_string") == 0) {
            (void)license_error_code_to_string(License_result_success);
        } else if (strcmp(func_name, "full_license_check") == 0) {
            (void)full_license_check();
        }
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        stats->measurements[i] = timespec_diff_ns(&start, &end);
        
        // 진행률 표시 (큰 반복 횟수일 때)
        if (iterations >= 100 && (i + 1) % (iterations / 10) == 0) {
            printf("  Progress: %d%% (%d/%d)\n", 
                   (i + 1) * 100 / iterations, i + 1, iterations);
        }
    }
    
    calculate_statistics(stats);
    return stats;
}

// 모든 함수의 통계 분석
static void comprehensive_function_analysis(int iterations) {
    printf("\n" COLOR_BOLD COLOR_MAGENTA "=== Comprehensive Function Analysis (%d iterations) ===" COLOR_RESET "\n", iterations);
    
    const char* functions[] = {
        "load_license_file",
        "get_license_expire_time", 
        "get_license_create_date",
        "is_license_valid_period",
        "license_error_code_to_string",
        "full_license_check"
    };
    
    int num_functions = sizeof(functions) / sizeof(functions[0]);
    function_stats_t** all_stats = malloc(num_functions * sizeof(function_stats_t*));
    
    // 각 함수별 성능 측정
    for (int i = 0; i < num_functions; i++) {
        all_stats[i] = measure_function_performance(functions[i], iterations);
        if (all_stats[i]) {
            print_function_statistics(all_stats[i]);
        }
    }
    
    // 비교 분석 테이블
    printf("\n" COLOR_BOLD COLOR_CYAN "=== Function Performance Comparison ===" COLOR_RESET "\n");
    printf("┌──────────────────────────────┬─────────────┬─────────────┬─────────────┬─────────────┐\n");
    printf("│ %-28s │ %-11s │ %-11s │ %-11s │ %-11s │\n", 
           "Function", "Mean (μs)", "Std Dev", "Min (μs)", "Max (μs)");
    printf("├──────────────────────────────┼─────────────┼─────────────┼─────────────┼─────────────┤\n");
    
    for (int i = 0; i < num_functions; i++) {
        if (all_stats[i]) {
            printf("│ %-28s │ %8.2f μs │ %8.2f μs │ %8.2f μs │ %8.2f μs │\n",
                   all_stats[i]->function_name,
                   NS_TO_US(all_stats[i]->mean),
                   NS_TO_US(all_stats[i]->std_dev),
                   NS_TO_US(all_stats[i]->min_time),
                   NS_TO_US(all_stats[i]->max_time));
        }
    }
    printf("└──────────────────────────────┴─────────────┴─────────────┴─────────────┴─────────────┘\n");
    
    // 메모리 정리
    for (int i = 0; i < num_functions; i++) {
        if (all_stats[i]) {
            free(all_stats[i]->measurements);
            free(all_stats[i]);
        }
    }
    free(all_stats);
}

// 스트레스 테스트 함수
static void stress_test(int test_iterations) {
    printf("\n" COLOR_BOLD COLOR_MAGENTA "=== Stress Test: %d iterations ===" COLOR_RESET "\n", test_iterations);
    
    struct timespec start, end;
    int success_count = 0;
    int failure_count = 0;
    long min_time = LONG_MAX;
    long max_time = 0;
    long total_time = 0;
    
    for (int i = 0; i < test_iterations; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        if (full_license_check()) {
            success_count++;
        } else {
            failure_count++;
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        long iteration_time = timespec_diff_ns(&start, &end);
        total_time += iteration_time;
        
        if (iteration_time < min_time) min_time = iteration_time;
        if (iteration_time > max_time) max_time = iteration_time;
        
        // 진행률 표시 (100회마다)
        if (test_iterations >= 100 && (i + 1) % (test_iterations / 10) == 0) {
            printf("Progress: %d%% (%d/%d)\n", 
                   (i + 1) * 100 / test_iterations, i + 1, test_iterations);
        }
    }
    
    double avg_time = (double)total_time / test_iterations;
    double checks_per_second = test_iterations / (total_time / 1000000000.0);
    
    printf("\n" COLOR_CYAN "Stress Test Results:" COLOR_RESET "\n");
    printf("  Iterations: %d\n", test_iterations);
    printf("  Success: " COLOR_GREEN "%d" COLOR_RESET " | Failure: " COLOR_RED "%d" COLOR_RESET "\n", 
           success_count, failure_count);
    printf("  Min time: %.2f μs\n", NS_TO_US(min_time));
    printf("  Max time: %.2f μs\n", NS_TO_US(max_time));
    printf("  Avg time: %.2f μs\n", NS_TO_US(avg_time));
    printf("  Throughput: %.0f checks/second\n", checks_per_second);
    printf("  Success rate: %.1f%%\n", (double)success_count / test_iterations * 100);
}

int main() {
    struct timespec start, end;
    long elapsed_ns;
    License_error_code code;
    time_t expired_time, publish_time;
    int is_valid;

    printf(COLOR_BOLD COLOR_CYAN "=== LibLicense Performance Test ===" COLOR_RESET "\n\n");

    // 1. load_license_file() 성능 측정
    printf(COLOR_YELLOW "1. Testing load_license_file()..." COLOR_RESET "\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    code = load_license_file();
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_diff_ns(&start, &end);
    print_performance("load_license_file()", elapsed_ns);

    if (code != License_result_success) {
        printf(COLOR_RED "License Load Error: %s (code: %d)" COLOR_RESET "\n", 
               license_error_code_to_string(code), code);
        return 1;
    }
    printf(COLOR_GREEN "✅ License loaded successfully\n" COLOR_RESET "\n");

    // 2. get_license_expire_time() 성능 측정
    printf(COLOR_YELLOW "2. Testing get_license_expire_time()..." COLOR_RESET "\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    expired_time = get_license_expire_time();
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_diff_ns(&start, &end);
    print_performance("get_license_expire_time()", elapsed_ns);

    // 3. get_license_create_date() 성능 측정
    printf(COLOR_YELLOW "3. Testing get_license_create_date()..." COLOR_RESET "\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    publish_time = get_license_create_date();
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_diff_ns(&start, &end);
    print_performance("get_license_create_date()", elapsed_ns);

    // 4. is_license_valid_period() 성능 측정
    printf(COLOR_YELLOW "4. Testing is_license_valid_period()..." COLOR_RESET "\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    is_valid = is_license_valid_period();
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_diff_ns(&start, &end);
    print_performance("is_license_valid_period()", elapsed_ns);

    // 5. license_error_code_to_string() 성능 측정
    printf(COLOR_YELLOW "5. Testing license_error_code_to_string()..." COLOR_RESET "\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    (void)license_error_code_to_string(License_result_success);  // void 캐스팅으로 unused 경고 제거
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_diff_ns(&start, &end);
    print_performance("license_error_code_to_string()", elapsed_ns);

    // 라이센스 정보 출력
    printf("\n" COLOR_BOLD COLOR_GREEN "=== License Information ===" COLOR_RESET "\n");
    printf("License Publish Time: %s", ctime(&publish_time));
    
    if (expired_time == 0) {
        printf("License Expiration Time: " COLOR_GREEN "Never (Unlimited)" COLOR_RESET "\n");
    } else {
        printf("License Expiration Time: %s", ctime(&expired_time));
        
        // 만료까지 남은 시간 계산
        time_t now = time(NULL);
        if (now < expired_time) {
            time_t remaining = expired_time - now;
            long days = remaining / (24 * 3600);
            long hours = (remaining % (24 * 3600)) / 3600;
            long minutes = (remaining % 3600) / 60;
            printf("Time Remaining: " COLOR_GREEN "%ld days, %ld hours, %ld minutes" COLOR_RESET "\n", 
                   days, hours, minutes);
        }
    }

    printf("License Status: ");
    if (is_valid) {
        printf(COLOR_GREEN "✅ VALID" COLOR_RESET "\n");
    } else {
        printf(COLOR_RED "❌ INVALID/EXPIRED" COLOR_RESET "\n");
        return 1;
    }

    // 반복 성능 테스트 (함수 호출 오버헤드 측정)
    printf("\n" COLOR_BOLD COLOR_CYAN "=== Repeated Performance Test (1000 calls) ===" COLOR_RESET "\n");
    
    const int iterations = 1000;
    
    // get_license_expire_time() 반복 테스트
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < iterations; i++) {
        get_license_expire_time();
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_diff_ns(&start, &end);
    printf("get_license_expire_time() x%d: %.2f ns/call\n", 
           iterations, (double)elapsed_ns / iterations);

    // is_license_valid_period() 반복 테스트
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < iterations; i++) {
        is_license_valid_period();
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_diff_ns(&start, &end);
    printf("is_license_valid_period() x%d: %.2f ns/call\n", 
           iterations, (double)elapsed_ns / iterations);

    // license_error_code_to_string() 반복 테스트
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < iterations; i++) {
        license_error_code_to_string(License_result_success);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_diff_ns(&start, &end);
    printf("license_error_code_to_string() x%d: %.2f ns/call\n", 
           iterations, (double)elapsed_ns / iterations);

    // 전체 라이센스 검증 프로세스 반복 테스트
    printf("\n" COLOR_BOLD COLOR_MAGENTA "=== Full License Check Process (1000 times) ===" COLOR_RESET "\n");
    
    int success_count = 0;
    int failure_count = 0;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < iterations; i++) {
        if (full_license_check()) {
            success_count++;
        } else {
            failure_count++;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_diff_ns(&start, &end);
    
    printf("Full license check x%d:\n", iterations);
    printf("  Total time: %.2f ms\n", NS_TO_MS(elapsed_ns));
    printf("  Average time per check: %.2f μs\n", NS_TO_US(elapsed_ns) / iterations);
    printf("  Success: " COLOR_GREEN "%d" COLOR_RESET " | Failure: " COLOR_RED "%d" COLOR_RESET "\n", 
           success_count, failure_count);
    
    if (success_count > 0) {
        printf("  " COLOR_GREEN "Success rate: %.1f%%" COLOR_RESET "\n", 
               (double)success_count / iterations * 100);
    }
    
    // 처리량 계산
    double checks_per_second = (double)iterations / (elapsed_ns / 1000000000.0);
    printf("  " COLOR_CYAN "Throughput: %.0f checks/second" COLOR_RESET "\n", checks_per_second);

    // 메모리 사용량 추정 (대략적)
    size_t memory_per_check = sizeof(License_generator_info) + 1024; // 추가 스택/힙 사용량 추정
    double total_memory_mb = (double)(memory_per_check * iterations) / (1024 * 1024);
    printf("  " COLOR_YELLOW "Estimated memory usage: %.2f MB" COLOR_RESET "\n", total_memory_mb);
    
    printf("\n" COLOR_BOLD COLOR_BLUE "=== Performance Summary ===" COLOR_RESET "\n");
    printf("┌─────────────────────────────────┬──────────────────┐\n");
    printf("│ %-31s │ %-16s │\n", "Operation", "Performance");
    printf("├─────────────────────────────────┼──────────────────┤\n");
    printf("│ %-31s │ %13.2f ms │\n", "Single license load", NS_TO_MS(elapsed_ns) / iterations);
    printf("│ %-31s │ %13.0f/sec │\n", "License checks", checks_per_second);
    printf("│ %-31s │ %13.1f%% │\n", "Success rate", (double)success_count / iterations * 100);
    printf("│ %-31s │ %13.2f MB │\n", "Memory usage (est.)", total_memory_mb);
    printf("└─────────────────────────────────┴──────────────────┘\n");

    // 추가 스트레스 테스트들
    stress_test(100);    // 100회 테스트
    stress_test(500);    // 500회 테스트  
    stress_test(1000);   // 1000회 테스트
    stress_test(5000);   // 5000회 테스트

    // 종합 함수 분석 (각 함수별 상세 통계)
    comprehensive_function_analysis(100);   // 100회 반복으로 각 함수 분석
    comprehensive_function_analysis(1000);  // 1000회 반복으로 각 함수 분석

    printf("\n" COLOR_BOLD COLOR_GREEN "=== All Performance Tests Completed Successfully ===" COLOR_RESET "\n");

    return 0;
}