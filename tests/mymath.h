// mymath.h - External C header for JustJIT inline C (with edge cases)
#ifndef MYMATH_H
#define MYMATH_H

// =========================================================================
// Constants
// =========================================================================
#define MY_PI     3.14159265358979323846
#define MY_E      2.71828182845904523536
#define MY_PHI    1.61803398874989484820  // Golden ratio
#define MY_SQRT2  1.41421356237309504880
#define MY_LN2    0.69314718055994530942

// Edge case constants
#define MY_EPSILON 1e-15
#define MY_INF     (1.0/0.0)
#define MY_NEG_INF (-1.0/0.0)

// =========================================================================
// Basic Math Functions
// =========================================================================
static inline double my_square(double x) {
    return x * x;
}

static inline double my_cube(double x) {
    return x * x * x;
}

static inline long long my_factorial(int n) {
    if (n < 0) return -1;  // Edge case: negative
    if (n <= 1) return 1;
    long long result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

// =========================================================================
// Edge Case Functions
// =========================================================================

// Safe division - handles divide by zero
static inline double my_safe_div(double a, double b) {
    if (b == 0.0) {
        if (a > 0) return MY_INF;
        if (a < 0) return MY_NEG_INF;
        return 0.0;  // 0/0 = 0 (by convention)
    }
    return a / b;
}

// Absolute value
static inline double my_abs(double x) {
    return x < 0 ? -x : x;
}

// Sign function: -1, 0, or 1
static inline int my_sign(double x) {
    if (x > MY_EPSILON) return 1;
    if (x < -MY_EPSILON) return -1;
    return 0;
}

// Clamp value to range
static inline double my_clamp(double x, double min_val, double max_val) {
    if (x < min_val) return min_val;
    if (x > max_val) return max_val;
    return x;
}

// Linear interpolation
static inline double my_lerp(double a, double b, double t) {
    return a + t * (b - a);
}

// Check if approximately equal
static inline int my_approx_eq(double a, double b, double epsilon) {
    return my_abs(a - b) < epsilon;
}

// =========================================================================
// Integer Edge Cases
// =========================================================================

// Greatest common divisor (Euclidean algorithm)
static inline long long my_gcd(long long a, long long b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        long long t = b;
        b = a % b;
        a = t;
    }
    return a;
}

// Least common multiple
static inline long long my_lcm(long long a, long long b) {
    if (a == 0 || b == 0) return 0;
    long long gcd = my_gcd(a, b);
    return (a / gcd) * b;  // Avoid overflow
}

// Power function for integers
static inline long long my_pow_int(long long base, int exp) {
    if (exp < 0) return 0;  // Edge case: negative exponent
    if (exp == 0) return 1;
    long long result = 1;
    while (exp > 0) {
        if (exp & 1) result *= base;
        base *= base;
        exp >>= 1;
    }
    return result;
}

// Check if prime
static inline int my_is_prime(long long n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

// =========================================================================
// Circle/Geometry
// =========================================================================
static inline double circle_area(double radius) {
    if (radius < 0) return -1.0;  // Edge case
    return MY_PI * radius * radius;
}

static inline double circle_circumference(double radius) {
    if (radius < 0) return -1.0;  // Edge case
    return 2.0 * MY_PI * radius;
}

static inline double sphere_volume(double radius) {
    if (radius < 0) return -1.0;
    return (4.0 / 3.0) * MY_PI * radius * radius * radius;
}

// =========================================================================
// Array/Memory Helpers
// =========================================================================

// Sum of array
static inline long long my_array_sum(long long* arr, int len) {
    if (!arr || len <= 0) return 0;
    long long sum = 0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum;
}

// Find max in array
static inline long long my_array_max(long long* arr, int len) {
    if (!arr || len <= 0) return 0;
    long long max = arr[0];
    for (int i = 1; i < len; i++) {
        if (arr[i] > max) max = arr[i];
    }
    return max;
}

#endif // MYMATH_H
