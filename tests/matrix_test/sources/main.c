#include <kore3/log.h>
#include <kore3/math/matrix.h>
#include <kore3/math/vector.h>
#include <kore3/system.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPSILON 0.001f
#define BENCHMARK_ITERATIONS 1000000

static int tests_passed = 0;
static int tests_failed = 0;

static void test_identity(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "Testing kore_matrix4x4_identity()...");
    
    kore_matrix4x4 id = kore_matrix4x4_identity();
    
    if (fabsf(kore_matrix4x4_get(&id, 0, 0) - 1.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [0][0]"); tests_failed++; return; }
    if (fabsf(kore_matrix4x4_get(&id, 1, 1) - 1.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [1][1]"); tests_failed++; return; }
    if (fabsf(kore_matrix4x4_get(&id, 2, 2) - 1.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [2][2]"); tests_failed++; return; }
    if (fabsf(kore_matrix4x4_get(&id, 3, 3) - 1.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [3][3]"); tests_failed++; return; }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i != j && fabsf(kore_matrix4x4_get(&id, i, j)) >= EPSILON) {
                kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [%d][%d] = %f (expected 0)", i, j, kore_matrix4x4_get(&id, i, j));
                tests_failed++;
                return;
            }
        }
    }
    
    kore_log(KORE_LOG_LEVEL_INFO, "  PASSED");
    tests_passed++;
}

static void test_rotation_x(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "Testing kore_matrix4x4_rotation_x()...");
    
    kore_matrix4x4 rot = kore_matrix4x4_rotation_x(3.14159f);
    
    if (fabsf(kore_matrix4x4_get(&rot, 0, 0) - 1.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [0][0]"); tests_failed++; return; }
    if (fabsf(kore_matrix4x4_get(&rot, 1, 1) - (-1.0f)) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [1][1]"); tests_failed++; return; }
    if (fabsf(kore_matrix4x4_get(&rot, 2, 2) - (-1.0f)) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [2][2]"); tests_failed++; return; }
    
    kore_log(KORE_LOG_LEVEL_INFO, "  PASSED");
    tests_passed++;
}

static void test_rotation_y(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "Testing kore_matrix4x4_rotation_y()...");
    
    kore_matrix4x4 rot = kore_matrix4x4_rotation_y(3.14159f);
    
    if (fabsf(kore_matrix4x4_get(&rot, 0, 0) - (-1.0f)) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [0][0]"); tests_failed++; return; }
    if (fabsf(kore_matrix4x4_get(&rot, 1, 1) - 1.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [1][1]"); tests_failed++; return; }
    if (fabsf(kore_matrix4x4_get(&rot, 2, 2) - (-1.0f)) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [2][2]"); tests_failed++; return; }
    
    kore_log(KORE_LOG_LEVEL_INFO, "  PASSED");
    tests_passed++;
}

static void test_rotation_z(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "Testing kore_matrix4x4_rotation_z()...");
    
    kore_matrix4x4 rot = kore_matrix4x4_rotation_z(3.14159f);
    
    if (fabsf(kore_matrix4x4_get(&rot, 0, 0) - (-1.0f)) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [0][0]"); tests_failed++; return; }
    if (fabsf(kore_matrix4x4_get(&rot, 1, 1) - (-1.0f)) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [1][1]"); tests_failed++; return; }
    if (fabsf(kore_matrix4x4_get(&rot, 2, 2) - 1.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [2][2]"); tests_failed++; return; }
    
    kore_log(KORE_LOG_LEVEL_INFO, "  PASSED");
    tests_passed++;
}

static void test_translation(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "Testing kore_matrix4x4_translation()...");
    
    kore_matrix4x4 trans = kore_matrix4x4_translation(3.0f, 4.0f, 5.0f);
    
    if (fabsf(kore_matrix4x4_get(&trans, 3, 0) - 3.0f) >= EPSILON) { 
        kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [3][0] = %f (expected 3.0)", kore_matrix4x4_get(&trans, 3, 0)); 
        tests_failed++; return; 
    }
    if (fabsf(kore_matrix4x4_get(&trans, 3, 1) - 4.0f) >= EPSILON) { 
        kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [3][1] = %f (expected 4.0)", kore_matrix4x4_get(&trans, 3, 1)); 
        tests_failed++; return; 
    }
    if (fabsf(kore_matrix4x4_get(&trans, 3, 2) - 5.0f) >= EPSILON) { 
        kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [3][2] = %f (expected 5.0)", kore_matrix4x4_get(&trans, 3, 2)); 
        tests_failed++; return; 
    }
    
    kore_log(KORE_LOG_LEVEL_INFO, "  PASSED");
    tests_passed++;
}

static void test_scale(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "Testing kore_matrix4x4_scale()...");
    
    kore_matrix4x4 sc = kore_matrix4x4_scale(2.0f, 3.0f, 4.0f);
    
    if (fabsf(kore_matrix4x4_get(&sc, 0, 0) - 2.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [0][0]"); tests_failed++; return; }
    if (fabsf(kore_matrix4x4_get(&sc, 1, 1) - 3.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [1][1]"); tests_failed++; return; }
    if (fabsf(kore_matrix4x4_get(&sc, 2, 2) - 4.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [2][2]"); tests_failed++; return; }
    
    kore_log(KORE_LOG_LEVEL_INFO, "  PASSED");
    tests_passed++;
}

static void test_multiply(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "Testing kore_matrix4x4_multiply()...");
    
    kore_matrix4x4 a = kore_matrix4x4_identity();
    kore_matrix4x4 b = kore_matrix4x4_translation(1.0f, 2.0f, 3.0f);
    
    kore_matrix4x4 result = kore_matrix4x4_multiply(&a, &b);
    
    if (fabsf(kore_matrix4x4_get(&result, 3, 0) - 1.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [3][0]"); tests_failed++; return; }
    if (fabsf(kore_matrix4x4_get(&result, 3, 1) - 2.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [3][1]"); tests_failed++; return; }
    if (fabsf(kore_matrix4x4_get(&result, 3, 2) - 3.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED at [3][2]"); tests_failed++; return; }
    
    kore_log(KORE_LOG_LEVEL_INFO, "  PASSED");
    tests_passed++;
}

static void test_multiply_vector(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "Testing kore_matrix4x4_multiply_vector()...");
    
    kore_matrix4x4 id = kore_matrix4x4_identity();
    kore_float4 v = {1.0f, 2.0f, 3.0f, 1.0f};
    kore_float4 result = kore_matrix4x4_multiply_vector(&id, v);
    
    if (fabsf(result.x - 1.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED: result.x = %f", result.x); tests_failed++; return; }
    if (fabsf(result.y - 2.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED: result.y = %f", result.y); tests_failed++; return; }
    if (fabsf(result.z - 3.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED: result.z = %f", result.z); tests_failed++; return; }
    
    kore_matrix4x4 trans = kore_matrix4x4_translation(1.0f, 2.0f, 3.0f);
    result = kore_matrix4x4_multiply_vector(&trans, v);
    
    if (fabsf(result.x - 2.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED trans: result.x = %f", result.x); tests_failed++; return; }
    if (fabsf(result.y - 4.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED trans: result.y = %f", result.y); tests_failed++; return; }
    if (fabsf(result.z - 6.0f) >= EPSILON) { kore_log(KORE_LOG_LEVEL_INFO, "  FAILED trans: result.z = %f", result.z); tests_failed++; return; }
    
    kore_log(KORE_LOG_LEVEL_INFO, "  PASSED");
    tests_passed++;
}

static int matrices_equal_detail(kore_matrix4x4 *a, kore_matrix4x4 *b, const char *test_name) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float av = kore_matrix4x4_get(a, i, j);
            float bv = kore_matrix4x4_get(b, i, j);
            if (fabsf(av - bv) >= EPSILON) {
                kore_log(KORE_LOG_LEVEL_INFO, "  %s MISMATCH at [%d][%d]: scalar=%.6f, simd=%.6f, diff=%.6f", 
                         test_name, i, j, av, bv, fabsf(av - bv));
                return 0;
            }
        }
    }
    return 1;
}

static int matrices_equal(kore_matrix4x4 *a, kore_matrix4x4 *b) {
    return matrices_equal_detail(a, b, "");
}

static void test_multiply_simd_correctness(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "Testing kore_matrix4x4_multiply_simd() correctness...");
    
    kore_matrix4x4 a = kore_matrix4x4_rotation_x(0.5f);
    kore_matrix4x4 b = kore_matrix4x4_translation(1.0f, 2.0f, 3.0f);
    
    kore_matrix4x4 result_scalar = kore_matrix4x4_multiply(&a, &b);
    kore_matrix4x4 result_simd = kore_matrix4x4_multiply_simd(&a, &b);
    
    if (!matrices_equal_detail(&result_scalar, &result_simd, "multiply")) {
        kore_log(KORE_LOG_LEVEL_INFO, "  FAILED: SIMD result differs from scalar");
        tests_failed++;
        return;
    }
    
    kore_matrix4x4 c = kore_matrix4x4_scale(2.0f, 3.0f, 4.0f);
    kore_matrix4x4 d = kore_matrix4x4_rotation_y(1.2f);
    
    result_scalar = kore_matrix4x4_multiply(&c, &d);
    result_simd = kore_matrix4x4_multiply_simd(&c, &d);
    
    if (!matrices_equal_detail(&result_scalar, &result_simd, "multiply2")) {
        kore_log(KORE_LOG_LEVEL_INFO, "  FAILED: SIMD result differs from scalar (test 2)");
        tests_failed++;
        return;
    }
    
    kore_log(KORE_LOG_LEVEL_INFO, "  PASSED");
    tests_passed++;
}

static void test_multiply_vector_simd_correctness(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "Testing kore_matrix4x4_multiply_vector_simd() correctness...");
    
    kore_matrix4x4 m = kore_matrix4x4_rotation_y(0.7f);
    kore_float4 v = {1.0f, 2.0f, 3.0f, 1.0f};
    
    kore_float4 result_scalar = kore_matrix4x4_multiply_vector(&m, v);
    kore_float4 result_simd = kore_matrix4x4_multiply_vector_simd(&m, v);
    
    if (fabsf(result_scalar.x - result_simd.x) >= EPSILON ||
        fabsf(result_scalar.y - result_simd.y) >= EPSILON ||
        fabsf(result_scalar.z - result_simd.z) >= EPSILON ||
        fabsf(result_scalar.w - result_simd.w) >= EPSILON) {
        kore_log(KORE_LOG_LEVEL_INFO, "  FAILED: SIMD result differs from scalar");
        tests_failed++;
        return;
    }
    
    kore_log(KORE_LOG_LEVEL_INFO, "  PASSED");
    tests_passed++;
}

static void test_transpose_simd_correctness(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "Testing kore_matrix4x4_transpose_simd() correctness...");
    
    kore_matrix4x4 m = kore_matrix4x4_rotation_x(0.3f);
    kore_matrix4x4 m_copy;
    memcpy(&m_copy, &m, sizeof(kore_matrix4x4));
    
    kore_matrix4x4_transpose(&m);
    kore_matrix4x4_transpose_simd(&m_copy);
    
    if (!matrices_equal(&m, &m_copy)) {
        kore_log(KORE_LOG_LEVEL_INFO, "  FAILED: SIMD result differs from scalar");
        tests_failed++;
        return;
    }
    
    kore_log(KORE_LOG_LEVEL_INFO, "  PASSED");
    tests_passed++;
}

static void benchmark_multiply(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "");
    kore_log(KORE_LOG_LEVEL_INFO, "=== Matrix Multiply Benchmark (%d iterations) ===", BENCHMARK_ITERATIONS);
    
    kore_matrix4x4 a = kore_matrix4x4_rotation_x(0.5f);
    kore_matrix4x4 b = kore_matrix4x4_translation(1.0f, 2.0f, 3.0f);
    kore_matrix4x4 result;
    
    double start = kore_timestamp();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        result = kore_matrix4x4_multiply(&a, &b);
    }
    double end = kore_timestamp();
    double scalar_ms = (end - start) * 1000.0;
    
    start = kore_timestamp();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        result = kore_matrix4x4_multiply_simd(&a, &b);
    }
    end = kore_timestamp();
    double simd_ms = (end - start) * 1000.0;
    
    double speedup = scalar_ms / simd_ms;
    
    kore_log(KORE_LOG_LEVEL_INFO, "  Scalar:  %.2f ms (%.2f M ops/sec)", scalar_ms, BENCHMARK_ITERATIONS / scalar_ms / 1000.0);
    kore_log(KORE_LOG_LEVEL_INFO, "  SIMD:    %.2f ms (%.2f M ops/sec)", simd_ms, BENCHMARK_ITERATIONS / simd_ms / 1000.0);
    kore_log(KORE_LOG_LEVEL_INFO, "  Speedup: %.2fx", speedup);
    
    (void)result;
}

static void benchmark_multiply_vector(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "");
    kore_log(KORE_LOG_LEVEL_INFO, "=== Matrix-Vector Multiply Benchmark (%d iterations) ===", BENCHMARK_ITERATIONS);
    
    kore_matrix4x4 m = kore_matrix4x4_rotation_y(0.7f);
    kore_float4 v = {1.0f, 2.0f, 3.0f, 1.0f};
    kore_float4 result;
    
    double start = kore_timestamp();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        result = kore_matrix4x4_multiply_vector(&m, v);
    }
    double end = kore_timestamp();
    double scalar_ms = (end - start) * 1000.0;
    
    start = kore_timestamp();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        result = kore_matrix4x4_multiply_vector_simd(&m, v);
    }
    end = kore_timestamp();
    double simd_ms = (end - start) * 1000.0;
    
    double speedup = scalar_ms / simd_ms;
    
    kore_log(KORE_LOG_LEVEL_INFO, "  Scalar:  %.2f ms (%.2f M ops/sec)", scalar_ms, BENCHMARK_ITERATIONS / scalar_ms / 1000.0);
    kore_log(KORE_LOG_LEVEL_INFO, "  SIMD:    %.2f ms (%.2f M ops/sec)", simd_ms, BENCHMARK_ITERATIONS / simd_ms / 1000.0);
    kore_log(KORE_LOG_LEVEL_INFO, "  Speedup: %.2fx", speedup);
    
    (void)result;
}

static void benchmark_transpose(void) {
    kore_log(KORE_LOG_LEVEL_INFO, "");
    kore_log(KORE_LOG_LEVEL_INFO, "=== Matrix Transpose Benchmark (%d iterations) ===", BENCHMARK_ITERATIONS);
    
    kore_matrix4x4 m_scalar = kore_matrix4x4_rotation_z(0.3f);
    kore_matrix4x4 m_simd = kore_matrix4x4_rotation_z(0.3f);
    
    double start = kore_timestamp();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        kore_matrix4x4_transpose(&m_scalar);
    }
    double end = kore_timestamp();
    double scalar_ms = (end - start) * 1000.0;
    
    start = kore_timestamp();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        kore_matrix4x4_transpose_simd(&m_simd);
    }
    end = kore_timestamp();
    double simd_ms = (end - start) * 1000.0;
    
    double speedup = scalar_ms / simd_ms;
    
    kore_log(KORE_LOG_LEVEL_INFO, "  Scalar:  %.2f ms (%.2f M ops/sec)", scalar_ms, BENCHMARK_ITERATIONS / scalar_ms / 1000.0);
    kore_log(KORE_LOG_LEVEL_INFO, "  SIMD:    %.2f ms (%.2f M ops/sec)", simd_ms, BENCHMARK_ITERATIONS / simd_ms / 1000.0);
    kore_log(KORE_LOG_LEVEL_INFO, "  Speedup: %.2fx", speedup);
}

int kickstart(int argc, char **argv) {
    kore_log(KORE_LOG_LEVEL_INFO, "");
    kore_log(KORE_LOG_LEVEL_INFO, "=== Matrix Library Tests ===");
    kore_log(KORE_LOG_LEVEL_INFO, "");
    
    test_identity();
    test_rotation_x();
    test_rotation_y();
    test_rotation_z();
    test_translation();
    test_scale();
    test_multiply();
    test_multiply_vector();
    
    kore_log(KORE_LOG_LEVEL_INFO, "");
    kore_log(KORE_LOG_LEVEL_INFO, "=== SIMD Correctness Tests ===");
    kore_log(KORE_LOG_LEVEL_INFO, "");
    
    test_multiply_simd_correctness();
    test_multiply_vector_simd_correctness();
    test_transpose_simd_correctness();
    
    kore_log(KORE_LOG_LEVEL_INFO, "");
    kore_log(KORE_LOG_LEVEL_INFO, "=== Performance Benchmarks ===");
    
    benchmark_multiply();
    benchmark_multiply_vector();
    benchmark_transpose();
    
    kore_log(KORE_LOG_LEVEL_INFO, "");
    kore_log(KORE_LOG_LEVEL_INFO, "=== Results: %d passed, %d failed ===", tests_passed, tests_failed);
    
    if (tests_failed > 0) {
        kore_log(KORE_LOG_LEVEL_ERROR, "SOME TESTS FAILED!");
        return 1;
    }
    
    kore_log(KORE_LOG_LEVEL_INFO, "ALL TESTS PASSED!");
    
    kore_stop();
    
    return 0;
}
