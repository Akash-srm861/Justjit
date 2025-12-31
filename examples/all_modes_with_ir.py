"""
JustJIT Complete Demo - All Modes with LLVM IR
===============================================

This file demonstrates:
1. All JIT compilation modes with their generated LLVM IR
2. Mixed-mode usage: calling typed-mode functions from object mode
3. inline_c and dump_ir functionality

Run with: python examples/all_modes_with_ir.py
"""

import justjit
from justjit import jit, inline_c, dump_ir, dump_c_ir
import numpy as np

# File to collect all IR
IR_OUTPUT_FILE = "all_modes_ir.txt"

def write_ir(mode_name, func, ir_content):
    """Write IR to file"""
    with open(IR_OUTPUT_FILE, "a") as f:
        f.write(f"\n{'='*70}\n")
        f.write(f"Mode: {mode_name}\n")
        f.write(f"Function: {func.__name__ if hasattr(func, '__name__') else func}\n")
        f.write(f"{'='*70}\n\n")
        f.write(ir_content if ir_content else "(No IR available)")
        f.write("\n\n")

# Clear the file
with open(IR_OUTPUT_FILE, "w") as f:
    f.write("JustJIT LLVM IR Output - All Modes\n")
    f.write("=" * 70 + "\n")

print("=" * 70)
print("JustJIT Complete Demo - All Modes with LLVM IR")
print("=" * 70)


# =============================================================================
# Part 1: All JIT Compilation Modes with IR Output
# =============================================================================

print("\n--- Part 1: All JIT Modes with LLVM IR ---\n")

# Mode 1: int - 64-bit integer mode
@jit(mode='int')
def int_add(a, b):
    return a + b

int_add(1, 2)  # Trigger compilation
ir = dump_ir(int_add)
write_ir("int (i64)", int_add, ir)
print(f"[int]        int_add(3, 5) = {int_add(3, 5)}")


# Mode 2: float - 64-bit float mode
@jit(mode='float')
def float_mul(a, b):
    return a * b

float_mul(1.0, 2.0)
ir = dump_ir(float_mul)
write_ir("float (f64)", float_mul, ir)
print(f"[float]      float_mul(2.5, 4.0) = {float_mul(2.5, 4.0)}")


# Mode 3: bool - Boolean mode
@jit(mode='bool')
def bool_or(a, b):
    return a or b

bool_or(True, False)
ir = dump_ir(bool_or)
write_ir("bool (i1)", bool_or, ir)
print(f"[bool]       bool_or(True, False) = {bool_or(True, False)}")


# Mode 4: int32 - 32-bit integer mode
@jit(mode='int32')
def int32_sub(a, b):
    return a - b

int32_sub(10, 3)
ir = dump_ir(int32_sub)
write_ir("int32 (i32)", int32_sub, ir)
print(f"[int32]      int32_sub(100, 30) = {int32_sub(100, 30)}")


# Mode 5: float32 - 32-bit float mode
@jit(mode='float32')
def float32_div(a, b):
    return a / b

float32_div(1.0, 2.0)
ir = dump_ir(float32_div)
write_ir("float32 (f32)", float32_div, ir)
print(f"[float32]    float32_div(10.0, 4.0) = {float32_div(10.0, 4.0)}")


# Mode 6: complex128 - Complex number mode
@jit(mode='complex128')
def complex_add(a, b):
    return a + b

complex_add(1+2j, 3+4j)
ir = dump_ir(complex_add)
write_ir("complex128 ({f64, f64})", complex_add, ir)
print(f"[complex128] complex_add(1+2j, 3+4j) = {complex_add(1+2j, 3+4j)}")


# Mode 7: complex64 - Single-precision complex
@jit(mode='complex64')
def complex64_mul(a, b):
    return a * b

complex64_mul(1+1j, 2+2j)
ir = dump_ir(complex64_mul)
write_ir("complex64 ({f32, f32})", complex64_mul, ir)
print(f"[complex64]  complex64_mul(1+1j, 2+2j) = {complex64_mul(1+1j, 2+2j)}")


# Mode 8: ptr - Pointer mode for array access
@jit(mode='ptr')
def ptr_get(arr, i):
    return arr[i]

test_arr = np.array([10.0, 20.0, 30.0, 40.0])
ptr_get(test_arr.ctypes.data, 0)
ir = dump_ir(ptr_get)
write_ir("ptr (array access)", ptr_get, ir)
print(f"[ptr]        ptr_get(arr, 2) = {ptr_get(test_arr.ctypes.data, 2)}")


# Mode 9: auto/object - Full Python object mode
@jit()
def object_concat(a, b):
    return a + b

object_concat("hello", " world")
ir = dump_ir(object_concat)
write_ir("auto/object (PyObject*)", object_concat, ir)
print(f"[auto]       object_concat('hi', ' there') = {object_concat('hi', ' there')}")


print("\nLLVM IR written to:", IR_OUTPUT_FILE)


# =============================================================================
# Part 2: Mixed Mode Usage - Using typed functions from object mode
# =============================================================================

print("\n--- Part 2: Mixed Mode Usage ---\n")

# Define typed-mode functions
@jit(mode='float')
def fast_square(x):
    return x * x

@jit(mode='float')
def fast_cube(x):
    return x * x * x

@jit(mode='int')
def fast_factorial_step(n, acc):
    return acc * n

# Now use these from Python (object mode)
def compute_polynomial(x):
    """Uses JIT float-mode functions from regular Python"""
    # These calls go through the JIT-compiled native code
    sq = fast_square(x)
    cu = fast_cube(x)
    return sq + cu + x

def manual_factorial(n):
    """Uses JIT int-mode function in a Python loop"""
    result = 1
    for i in range(1, n + 1):
        result = fast_factorial_step(i, result)
    return result

print("Mixed mode: compute_polynomial(3.0)")
print(f"  fast_square(3.0) = {fast_square(3.0)}")
print(f"  fast_cube(3.0) = {fast_cube(3.0)}")
print(f"  Result: {compute_polynomial(3.0)}")

print("\nMixed mode: manual_factorial(5)")
print(f"  Using fast_factorial_step in Python loop")
print(f"  Result: {manual_factorial(5)}")


# Mixed complex operations
@jit(mode='complex128')
def complex_square(z):
    return z * z

def complex_polynomial(z):
    """Object-mode function calling complex128-mode function"""
    z2 = complex_square(z)
    return z2 + z + (1+0j)

print("\nMixed mode: complex_polynomial(2+3j)")
print(f"  complex_square(2+3j) = {complex_square(2+3j)}")
print(f"  Result: {complex_polynomial(2+3j)}")


# =============================================================================
# Part 3: inline_c Integration
# =============================================================================

print("\n--- Part 3: inline_c with LLVM IR ---\n")

try:
    # C functions
    c_funcs = inline_c('''
        double c_fast_exp_approx(double x) {
            // Taylor series approximation: 1 + x + x^2/2 + x^3/6
            double x2 = x * x;
            double x3 = x2 * x;
            return 1.0 + x + x2/2.0 + x3/6.0;
        }
        
        int c_gcd(int a, int b) {
            while (b != 0) {
                int temp = b;
                b = a % b;
                a = temp;
            }
            return a;
        }
    ''', dump_ir=True)
    
    # Get C IR
    c_ir = dump_c_ir()
    write_ir("inline_c (C functions)", "c_fast_exp_approx, c_gcd", c_ir)
    
    print(f"C function: c_fast_exp_approx(0.5) = {c_funcs['c_fast_exp_approx'](0.5):.6f}")
    print(f"C function: c_gcd(48, 18) = {c_funcs['c_gcd'](48, 18)}")
    
    # Use C function from Python
    def hybrid_compute(x):
        """Python function calling C function"""
        approx = c_funcs['c_fast_exp_approx'](x)
        return approx * 2
    
    print(f"\nHybrid: Python calling C function")
    print(f"  hybrid_compute(0.5) = {hybrid_compute(0.5):.6f}")
    
except RuntimeError as e:
    print(f"inline_c not available: {e}")
    write_ir("inline_c", "N/A", f"Error: {e}")


# =============================================================================
# Part 4: Generators
# =============================================================================

print("\n--- Part 4: JIT Generators ---\n")

@jit
def countdown(n):
    while n > 0:
        yield n
        n = n - 1

# Use generator from object mode
def count_with_jit():
    """Object-mode function consuming JIT generator"""
    total = 0
    for x in countdown(5):
        total = total + x
    return total

print(f"Generator: countdown(5) = {list(countdown(5))}")
print(f"Consuming generator in Python: sum = {count_with_jit()}")


# =============================================================================
# Part 5: All Modes Interop - Cross-mode function calls
# =============================================================================

print("\n--- Part 5: All Modes Interop ---\n")

# Define functions in ALL modes
@jit(mode='int')
def int_double(x):
    return x + x

@jit(mode='float')
def float_half(x):
    return x / 2.0

@jit(mode='int32')
def int32_square(x):
    return x * x

@jit(mode='float32')
def float32_cube(x):
    return x * x * x

@jit(mode='complex128')
def complex_conjugate(z):
    # Note: This returns z * 1 (identity) since we can't directly access real/imag
    # In practice you'd use more complex operations
    return z + z

@jit(mode='bool')
def bool_is_valid(a, b):
    return a and b

@jit(mode='ptr')
def ptr_sum_two(arr, i):
    # Returns arr[i] (to keep it simple)
    return arr[i]


print("=== Interop Demo: Chain of mode transitions ===\n")

# 1. Start with int mode -> pass to float mode
print("1. int -> float chain:")
int_result = int_double(5)  # int mode: 5 * 2 = 10
float_result = float_half(float(int_result))  # float mode: 10.0 / 2 = 5.0
print(f"   int_double(5) = {int_result}")
print(f"   float_half({float(int_result)}) = {float_result}")

# 2. Float -> int32 -> float32 chain
print("\n2. float -> int32 -> float32 chain:")
f1 = float_half(20.0)  # 10.0
i32 = int32_square(int(f1))  # 10^2 = 100
f32 = float32_cube(float(i32) / 10)  # 10^3 = 1000
print(f"   float_half(20.0) = {f1}")
print(f"   int32_square({int(f1)}) = {i32}")
print(f"   float32_cube({float(i32)/10}) = {f32}")

# 3. Complex mode operations
print("\n3. complex128 operations:")
z1 = 1 + 2j
z2 = 3 + 4j
c_result = complex_add(z1, z2)
c_doubled = complex_conjugate(c_result)
print(f"   complex_add({z1}, {z2}) = {c_result}")
print(f"   complex_conjugate({c_result}) = {c_doubled}")

# 4. Bool mode for validation
print("\n4. bool mode validation:")
val1 = bool_is_valid(True, True)
val2 = bool_is_valid(True, False)
print(f"   bool_is_valid(True, True) = {val1}")
print(f"   bool_is_valid(True, False) = {val2}")

# 5. Ptr mode with numpy array
print("\n5. ptr mode array access:")
data = np.array([100.0, 200.0, 300.0, 400.0, 500.0])
for i in range(5):
    val = ptr_sum_two(data.ctypes.data, i)
    print(f"   ptr_sum_two(data, {i}) = {val}")

# 6. object mode orchestrating ALL other modes
print("\n6. Object mode orchestrating all modes:")

def all_modes_pipeline(start_int, complex_input, arr_input):
    """
    This Python function (object mode) calls functions from ALL JIT modes:
    - int mode: int_double
    - float mode: float_half
    - int32 mode: int32_square
    - float32 mode: float32_cube
    - complex128 mode: complex_add, complex_conjugate
    - bool mode: bool_is_valid
    - ptr mode: ptr_sum_two
    """
    results = {}
    
    # int mode
    results['int'] = int_double(start_int)
    
    # float mode
    results['float'] = float_half(float(results['int']))
    
    # int32 mode
    results['int32'] = int32_square(int(results['float']))
    
    # float32 mode
    results['float32'] = float32_cube(1.5)
    
    # complex128 mode
    results['complex128'] = complex_add(complex_input, 1+1j)
    
    # bool mode
    results['bool'] = bool_is_valid(results['int'] > 0, results['float'] > 0)
    
    # ptr mode
    results['ptr'] = ptr_sum_two(arr_input.ctypes.data, 0)
    
    return results

# Run the pipeline
pipeline_data = np.array([999.0, 888.0, 777.0])
pipeline_results = all_modes_pipeline(7, 2+3j, pipeline_data)

print("   Pipeline input: start_int=7, complex=2+3j, arr=[999, 888, 777]")
print("   Pipeline results:")
for mode, result in pipeline_results.items():
    print(f"      [{mode:10s}] = {result}")


# 7. Mode conversion chain - data flows through all numeric modes
print("\n7. Data flow through all numeric modes:")

def numeric_mode_chain(n):
    """Data flows: int -> float -> int32 -> float32 -> back to Python"""
    step1 = int_double(n)           # int64: n * 2
    step2 = float_half(float(step1))  # f64: (n*2) / 2 = n
    step3 = int32_square(int(step2))  # i32: n^2
    step4 = float32_cube(step3 / 100.0)  # f32: ((n^2)/100)^3
    return step4

n = 10
print(f"   Input: n = {n}")
print(f"   Step 1 (int64):   int_double({n}) = {int_double(n)}")
print(f"   Step 2 (float64): float_half({float(int_double(n))}) = {float_half(float(int_double(n)))}")
print(f"   Step 3 (int32):   int32_square({int(float_half(float(int_double(n))))}) = {int32_square(int(float_half(float(int_double(n)))))}")
final = numeric_mode_chain(n)
print(f"   Step 4 (float32): float32_cube({int32_square(int(float_half(float(int_double(n)))))/100}) = {final}")
print(f"   Final result: {final}")


# 8. Complex + Array interop
print("\n8. Complex numbers with array data:")

complex_arr = np.array([1.0, 2.0, 3.0, 4.0])  # Pretend these are real parts

def complex_array_sum():
    """Combine ptr mode array access with complex mode operations"""
    # Get values from array using ptr mode
    real1 = ptr_sum_two(complex_arr.ctypes.data, 0)
    real2 = ptr_sum_two(complex_arr.ctypes.data, 1)
    
    # Create complex numbers and use complex128 mode
    c1 = complex(real1, real2)  # 1+2j
    c2 = complex(real2, real1)  # 2+1j
    
    return complex_add(c1, c2)

result = complex_array_sum()
print(f"   Array values: {list(complex_arr[:2])}")
print(f"   Created: c1 = 1+2j, c2 = 2+1j")
print(f"   complex_add result: {result}")


print("\n" + "=" * 70)
print("All Modes Interop Complete!")
print("=" * 70)


# =============================================================================
# Summary
# =============================================================================

print("\n" + "=" * 70)
print("Summary: All IR written to", IR_OUTPUT_FILE)
print("=" * 70)
print("""
Modes demonstrated:
  1. int        - 64-bit integer (i64)
  2. float      - 64-bit float (f64)
  3. bool       - Boolean (i1)
  4. int32      - 32-bit integer (i32)
  5. float32    - 32-bit float (f32)
  6. complex128 - Complex ({f64, f64})
  7. complex64  - Complex ({f32, f32})
  8. ptr        - Pointer (array access)
  9. auto       - Object mode (PyObject*)

Interop patterns demonstrated:
  - int -> float -> int32 -> float32 chains
  - Object mode calling all typed modes
  - Complex mode with array data
  - Bool mode for validation logic
  - Full data pipeline through all modes

Mixed-mode features:
  - Call typed-mode functions from Python
  - Use JIT generators in Python loops
  - Hybrid Python + inline_c workflows
""")
print("=" * 70)
