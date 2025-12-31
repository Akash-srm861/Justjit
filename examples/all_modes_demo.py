"""
JustJIT Complete Demo - All Modes and Features
===============================================

This file demonstrates all JIT compilation modes, inline C, and dump_ir functionality.
Run with: python examples/all_modes_demo.py
"""

import justjit
from justjit import jit, inline_c, dump_ir, dump_c_ir

print("=" * 70)
print("JustJIT Complete Demo - All Modes and Features")
print("=" * 70)


# =============================================================================
# Part 1: All JIT Compilation Modes
# =============================================================================

print("\n--- Part 1: JIT Compilation Modes ---\n")

# Mode 1: auto (default) - Full Python object mode
@jit()
def auto_add(a, b):
    return a + b

print(f"[auto]     auto_add(1, 2) = {auto_add(1, 2)}")
print(f"[auto]     auto_add('hello', ' world') = {auto_add('hello', ' world')}")


# Mode 2: int - 64-bit integer mode
@jit(mode='int')
def int_multiply(a, b):
    return a * b

print(f"[int]      int_multiply(6, 7) = {int_multiply(6, 7)}")


# Mode 3: float - 64-bit float mode
@jit(mode='float')
def float_divide(a, b):
    return a / b

print(f"[float]    float_divide(22.0, 7.0) = {float_divide(22.0, 7.0):.6f}")


# Mode 4: bool - Boolean mode
@jit(mode='bool')
def bool_and(a, b):
    return a and b

print(f"[bool]     bool_and(True, False) = {bool_and(True, False)}")


# Mode 5: int32 - 32-bit integer mode (C interop)
@jit(mode='int32')
def int32_add(a, b):
    return a + b

print(f"[int32]    int32_add(100, 200) = {int32_add(100, 200)}")


# Mode 6: float32 - 32-bit float mode (SIMD/ML)
@jit(mode='float32')
def float32_mul(a, b):
    return a * b

print(f"[float32]  float32_mul(1.5, 2.0) = {float32_mul(1.5, 2.0)}")


# Mode 7: complex128 - Complex number mode
@jit(mode='complex128')
def complex_add(a, b):
    return a + b

print(f"[complex128] complex_add((1+2j), (3+4j)) = {complex_add(1+2j, 3+4j)}")


# Mode 8: complex64 - Single-precision complex
@jit(mode='complex64')
def complex64_mul(a, b):
    return a * b

print(f"[complex64]  complex64_mul((1+1j), (2+2j)) = {complex64_mul(1+1j, 2+2j)}")


# Mode 9: ptr - Pointer mode for array access
# Note: Designed for array indexing with raw pointers
import numpy as np

@jit(mode='ptr')
def ptr_array_get(arr, i):
    return arr[i]

test_arr = np.array([10.0, 20.0, 30.0, 40.0])
print(f"[ptr]      ptr_array_get(arr, 2) = {ptr_array_get(test_arr.ctypes.data, 2)}")


# Mode 10: vec4f - SSE SIMD mode (<4 x f32>)
# Note: Requires special packed SIMD input - demo only shows concept
# @jit(mode='vec4f')
# def vec4f_add(a, b):
#     return a + b

print(f"[vec4f]    (SIMD mode - requires packed vector input)")


# Mode 11: vec8i - AVX SIMD mode (<8 x i32>)
# @jit(mode='vec8i')
# def vec8i_add(a, b):
#     return a + b

print(f"[vec8i]    (SIMD mode - requires packed vector input)")



# Mode 12: optional_f64 - Nullable float64
# @jit(mode='optional_f64')
# def optional_double(a, b):
#     return a + b

print(f"[optional_f64] (Optional mode - requires struct input)")


# =============================================================================
# Part 2: dump_ir - Inspecting Generated LLVM IR
# =============================================================================

print("\n--- Part 2: dump_ir - LLVM IR Inspection ---\n")

@jit(mode='float')
def simple_add(a, b):
    return a + b

# Trigger compilation
simple_add(1.0, 2.0)

# Get and print the LLVM IR
ir = dump_ir(simple_add)
print("LLVM IR for simple_add (float mode):")
print("-" * 40)
# Print first 500 chars to keep output manageable
print(ir[:500] if len(ir) > 500 else ir)
print("-" * 40)


# =============================================================================
# Part 3: Generators and Async
# =============================================================================

print("\n--- Part 3: JIT Generators ---\n")

@jit
def countdown(n):
    while n > 0:
        yield n
        n = n - 1

print("countdown(5):", list(countdown(5)))


# =============================================================================
# Part 4: inline_c - Compile C Code at Runtime
# =============================================================================

print("\n--- Part 4: inline_c - Runtime C Compilation ---\n")

try:
    # Basic C function
    result = inline_c('''
        int c_add(int a, int b) {
            return a + b;
        }
        
        double c_multiply(double x, double y) {
            return x * y;
        }
    ''')
    
    print(f"C functions compiled: {result['functions']}")
    print(f"c_add(10, 20) = {result['c_add'](10, 20)}")
    print(f"c_multiply(3.5, 2.0) = {result['c_multiply'](3.5, 2.0)}")
    
    # C with math.h
    math_result = inline_c('''
        #include <math.h>
        
        double compute_sqrt(double x) {
            return sqrt(x);
        }
        
        double compute_sin(double x) {
            return sin(x);
        }
    ''')
    
    print(f"compute_sqrt(16.0) = {math_result['compute_sqrt'](16.0)}")
    print(f"compute_sin(3.14159/2) = {math_result['compute_sin'](3.14159/2):.6f}")
    
except RuntimeError as e:
    print(f"inline_c not available: {e}")
    print("(This requires Clang support to be compiled in)")


# =============================================================================
# Part 5: dump_c_ir - Inspecting C-generated LLVM IR
# =============================================================================

print("\n--- Part 5: dump_c_ir - C Code LLVM IR ---\n")

try:
    result = inline_c('''
        double square(double x) {
            return x * x;
        }
    ''', dump_ir=True)
    
    c_ir = dump_c_ir()
    print("LLVM IR for square(double x):")
    print("-" * 40)
    # Print first 800 chars
    print(c_ir[:800] if len(c_ir) > 800 else c_ir)
    print("-" * 40)
    
except RuntimeError as e:
    print(f"dump_c_ir not available: {e}")


# =============================================================================
# Part 6: Performance Demo - Loop Optimization
# =============================================================================

print("\n--- Part 6: Performance Demo ---\n")

import time

@jit(mode='int')
def sum_range_jit(n):
    total = 0
    for i in range(n):
        total = total + i
    return total

def sum_range_python(n):
    total = 0
    for i in range(n):
        total = total + i
    return total

N = 1_000_000

# JIT version
start = time.perf_counter()
jit_result = sum_range_jit(N)
jit_time = time.perf_counter() - start

# Python version
start = time.perf_counter()
py_result = sum_range_python(N)
py_time = time.perf_counter() - start

print(f"sum_range({N:,}):")
print(f"  JIT:    {jit_result:,} in {jit_time*1000:.2f}ms")
print(f"  Python: {py_result:,} in {py_time*1000:.2f}ms")
print(f"  Speedup: {py_time/jit_time:.1f}x")


# =============================================================================
# Summary
# =============================================================================

print("\n" + "=" * 70)
print("All JIT Modes:")
print("  1. auto      - Full Python object mode (default)")
print("  2. int       - 64-bit integer (i64)")
print("  3. float     - 64-bit float (f64)")
print("  4. bool      - Boolean (i1)")
print("  5. int32     - 32-bit integer (i32)")
print("  6. float32   - 32-bit float (f32)")
print("  7. complex128 - Complex number ({f64, f64})")
print("  8. complex64  - Single-precision complex ({f32, f32})")
print("  9. ptr       - Pointer mode for array access")
print(" 10. vec4f     - SSE SIMD (<4 x f32>)")
print(" 11. vec8i     - AVX SIMD (<8 x i32>)")
print(" 12. optional_f64 - Nullable float64 ({i64, f64})")
print("")
print("Additional Features:")
print("  - dump_ir()    - Inspect LLVM IR from JIT-compiled Python")
print("  - inline_c()   - Compile C/C++ code at runtime")
print("  - dump_c_ir()  - Inspect LLVM IR from C compilation")
print("  - Generators   - State machine compilation")
print("  - Async/Await  - Coroutine support")
print("=" * 70)
