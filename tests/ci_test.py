"""
JustJIT CI Test Script - Comprehensive
=======================================

Complete validation tests for wheel installation.
Includes all features from robust_interop_test.py.
Used by CI after building and repairing wheels.

Run: python tests/ci_test.py
Exit code 0 = success, non-zero = failure
"""

import sys

def main():
    passed = 0
    failed = 0

    def check(name, actual, expected):
        nonlocal passed, failed
        if actual == expected:
            print(f"  [OK] {name}")
            passed += 1
            return True
        else:
            print(f"  [FAIL] {name}: got {actual}, expected {expected}")
            failed += 1
            return False

    def check_close(name, actual, expected, tol=1e-6):
        nonlocal passed, failed
        if abs(actual - expected) < tol:
            print(f"  [OK] {name}")
            passed += 1
            return True
        else:
            print(f"  [FAIL] {name}: got {actual}, expected {expected}")
            failed += 1
            return False

    print("=" * 70)
    print("JustJIT CI Test Suite - Comprehensive")
    print("=" * 70)

    # =========================================================================
    # Test 1: Basic import
    # =========================================================================
    print("\n--- Test 1: Import ---")
    try:
        import justjit
        from justjit import jit, dump_ir
        print("  [OK] justjit imported")
        passed += 1
    except ImportError as e:
        print(f"  [FAIL] Import failed: {e}")
        failed += 1
        sys.exit(1)

    # =========================================================================
    # Test 2: All JIT Modes
    # =========================================================================
    print("\n--- Test 2: All JIT Modes ---")

    # int mode (i64)
    @jit(mode='int')
    def int_add(a, b):
        return a + b

    @jit(mode='int')
    def int_double(x):
        return x + x

    check("int add", int_add(3, 5), 8)
    check("int double", int_double(7), 14)

    # float mode (f64)
    @jit(mode='float')
    def float_mul(a, b):
        return a * b

    @jit(mode='float')
    def float_square(x):
        return x * x

    check("float mul", float_mul(2.5, 4.0), 10.0)
    check("float square", float_square(3.0), 9.0)

    # int32 mode (i32)
    @jit(mode='int32')
    def int32_sub(a, b):
        return a - b

    @jit(mode='int32')
    def int32_cube(x):
        return x * x * x

    check("int32 sub", int32_sub(100, 30), 70)
    check("int32 cube", int32_cube(3), 27)

    # float32 mode (f32)
    @jit(mode='float32')
    def float32_div(a, b):
        return a / b

    @jit(mode='float32')
    def float32_half(x):
        return x / 2.0

    check("float32 div", float32_div(10.0, 4.0), 2.5)
    check("float32 half", float32_half(8.0), 4.0)

    # complex128 mode
    @jit(mode='complex128')
    def complex_add(a, b):
        return a + b

    @jit(mode='complex128')
    def complex_mul(a, b):
        return a * b

    check("complex128 add", complex_add(1+2j, 3+4j), 4+6j)
    check("complex128 mul", complex_mul(1+1j, 2+2j), 4j)

    # complex64 mode
    @jit(mode='complex64')
    def complex64_add(a, b):
        return a + b

    check("complex64 add", complex64_add(1+2j, 3+4j), 4+6j)

    # bool mode
    @jit(mode='bool')
    def bool_and(a, b):
        return a and b

    @jit(mode='bool')
    def bool_or(a, b):
        return a or b

    check("bool and TT", bool_and(True, True), True)
    check("bool and TF", bool_and(True, False), False)
    check("bool or TF", bool_or(True, False), True)
    check("bool or FT", bool_or(False, True), True)
    check("bool or FF", bool_or(False, False), False)

    # object mode
    @jit()
    def object_concat(a, b):
        return a + b

    check("object concat", object_concat("Hello", " World"), "Hello World")

    # =========================================================================
    # Test 3: Factorial (multi-step)
    # =========================================================================
    print("\n--- Test 3: Factorial ---")

    @jit(mode='int')
    def factorial_step(n, acc):
        return n * acc

    def factorial(n):
        result = 1
        for i in range(1, n + 1):
            result = factorial_step(i, result)
        return result

    check("factorial(0)", factorial(0), 1)
    check("factorial(1)", factorial(1), 1)
    check("factorial(5)", factorial(5), 120)
    check("factorial(10)", factorial(10), 3628800)

    # =========================================================================
    # Test 4: Mode Chains (interop)
    # =========================================================================
    print("\n--- Test 4: Mode Chains ---")

    # int -> float chain
    int_result = int_double(5)
    float_result = float_square(float(int_result))
    check("int->float chain", float_result, 100.0)

    # float -> int32 chain
    f1 = float_mul(3.0, 3.0)  # 9.0
    i32 = int32_cube(int(f1))  # 729
    check("float->int32 chain", i32, 729)

    # int32 -> float32 chain
    i32_val = int32_cube(2)  # 8
    f32_val = float32_half(float(i32_val))  # 4.0
    check("int32->float32 chain", f32_val, 4.0)

    # =========================================================================
    # Test 5: LLVM IR Generation
    # =========================================================================
    print("\n--- Test 5: LLVM IR Generation ---")
    ir = dump_ir(int_add)
    if ir and "define" in ir and "i64" in ir:
        print("  [OK] int mode IR generated")
        passed += 1
    else:
        print("  [FAIL] int mode IR not generated")
        failed += 1

    ir = dump_ir(float_mul)
    if ir and "define" in ir and "double" in ir:
        print("  [OK] float mode IR generated")
        passed += 1
    else:
        print("  [FAIL] float mode IR not generated")
        failed += 1

    # =========================================================================
    # Test 6: inline_c
    # =========================================================================
    print("\n--- Test 6: inline_c ---")
    HAS_INLINE_C = False
    try:
        from justjit import inline_c, dump_c_ir
        c_funcs = inline_c('''
            double c_square(double x) { return x * x; }
            double c_cube(double x) { return x * x * x; }
            double c_add(double a, double b) { return a + b; }
            int c_gcd(int a, int b) {
                while (b != 0) { int t = b; b = a % b; a = t; }
                return a;
            }
        ''')
        HAS_INLINE_C = True
        check("C square", c_funcs['c_square'](5.0), 25.0)
        check("C cube", c_funcs['c_cube'](3.0), 27.0)
        check("C add", c_funcs['c_add'](10.0, 5.0), 15.0)
        check("C gcd", c_funcs['c_gcd'](48, 18), 6)

        # C -> JIT chain
        c_sq = c_funcs['c_square'](4.0)  # 16
        jit_sq = float_square(c_sq)  # 256
        check("C->JIT chain", jit_sq, 256.0)

        # JIT -> C chain
        j_val = float_mul(3.0, 3.0)  # 9
        c_val = c_funcs['c_cube'](j_val)  # 729
        check("JIT->C chain", c_val, 729.0)

        # JIT -> C -> JIT chain
        step1 = int_double(3)  # 6
        step2 = c_funcs['c_cube'](float(step1))  # 216
        step3 = float32_half(step2)  # 108
        check("JIT->C->JIT chain", step3, 108.0)

    except RuntimeError as e:
        print(f"  [SKIP] inline_c not available: {e}")
    except Exception as e:
        print(f"  [FAIL] inline_c error: {e}")
        failed += 1

    # =========================================================================
    # Test 7: ptr mode with numpy
    # =========================================================================
    print("\n--- Test 7: ptr mode ---")
    try:
        import numpy as np

        @jit(mode='ptr')
        def ptr_get(arr, i):
            return arr[i]

        test_arr = np.array([10.0, 20.0, 30.0, 40.0, 50.0])
        check("ptr get [0]", ptr_get(test_arr.ctypes.data, 0), 10.0)
        check("ptr get [2]", ptr_get(test_arr.ctypes.data, 2), 30.0)
        check("ptr get [4]", ptr_get(test_arr.ctypes.data, 4), 50.0)

        # ptr -> JIT chain
        arr_val = ptr_get(test_arr.ctypes.data, 1)  # 20.0
        jit_val = float_square(arr_val)  # 400.0
        check("ptr->JIT chain", jit_val, 400.0)

        # ptr -> C -> JIT (if inline_c available)
        if HAS_INLINE_C:
            arr_val = ptr_get(test_arr.ctypes.data, 2)  # 30.0
            c_val = c_funcs['c_square'](arr_val)  # 900.0
            jit_val = float_square(c_val)  # 810000.0
            check("ptr->C->JIT chain", jit_val, 810000.0)

    except ImportError:
        print("  [SKIP] numpy not available")
    except Exception as e:
        print(f"  [FAIL] ptr mode error: {e}")
        failed += 1

    # =========================================================================
    # Test 8: GIL and RAII Wrappers
    # =========================================================================
    print("\n--- Test 8: GIL and RAII Wrappers ---")
    if HAS_INLINE_C:
        try:
            raii_funcs = inline_c('''
                extern void* jit_gil_acquire(void);
                extern void jit_gil_release(void* guard);
                extern void* jit_gil_release_begin(void);
                extern void jit_gil_release_end(void* save);
                extern double jit_py_to_double(void* obj);
                extern void* jit_double_to_py(double val);
                extern void jit_decref(void* obj);
                extern void* jit_long_to_py(long long val);
                extern void jit_incref(void* obj);

                int test_gil_cycle(void) {
                    void* guard = jit_gil_acquire();
                    if (!guard) return 0;
                    int result = 42;
                    jit_gil_release(guard);
                    return result;
                }

                double test_gil_release_parallel(double a, double b) {
                    void* save = jit_gil_release_begin();
                    double result = 0.0;
                    for (int i = 0; i < 1000; i++) {
                        result += a * b;
                    }
                    jit_gil_release_end(save);
                    return result / 1000.0;
                }

                double test_type_conversion(double val) {
                    void* py_val = jit_double_to_py(val);
                    if (!py_val) return -1.0;
                    double result = jit_py_to_double(py_val);
                    jit_decref(py_val);
                    return result;
                }

                int test_refcount(void) {
                    void* obj = jit_long_to_py(12345);
                    if (!obj) return 0;
                    jit_incref(obj);
                    jit_decref(obj);
                    jit_decref(obj);
                    return 1;
                }
            ''')

            check("GIL cycle", raii_funcs['test_gil_cycle'](), 42)
            check("GIL release parallel", raii_funcs['test_gil_release_parallel'](3.0, 4.0), 12.0)
            check_close("type conversion", raii_funcs['test_type_conversion'](3.14159), 3.14159)
            check("refcount", raii_funcs['test_refcount'](), 1)

        except Exception as e:
            print(f"  [FAIL] GIL/RAII test error: {e}")
            failed += 1
    else:
        print("  [SKIP] inline_c not available")

    # =========================================================================
    # Test 9: JIT Generators
    # =========================================================================
    print("\n--- Test 9: JIT Generators ---")
    try:
        @jit
        def countdown(n):
            while n > 0:
                yield n
                n = n - 1

        result = list(countdown(5))
        check("generator countdown", result, [5, 4, 3, 2, 1])

        total = sum(countdown(5))
        check("generator sum", total, 15)

    except Exception as e:
        print(f"  [FAIL] Generator error: {e}")
        failed += 1

    # =========================================================================
    # Test 10: Grand Pipeline (all modes + C)
    # =========================================================================
    print("\n--- Test 10: Grand Pipeline ---")
    try:
        if HAS_INLINE_C:
            # Step 1: int-JIT
            step1 = int_double(2)  # 4
            # Step 2: C cube
            step2 = c_funcs['c_cube'](float(step1))  # 64
            # Step 3: float-JIT square
            step3 = float_square(step2)  # 4096
            # Step 4: float32-JIT half
            step4 = float32_half(step3)  # 2048
            # Step 5: complex-JIT
            step5 = complex_add(3+4j, 1+2j)  # 4+6j

            check("pipeline step1 (int)", step1, 4)
            check("pipeline step2 (C)", step2, 64.0)
            check("pipeline step3 (float)", step3, 4096.0)
            check("pipeline step4 (float32)", step4, 2048.0)
            check("pipeline step5 (complex)", step5, 4+6j)
            print("  [OK] Grand pipeline complete")
            passed += 1
        else:
            print("  [SKIP] inline_c not available for full pipeline")

    except Exception as e:
        print(f"  [FAIL] Pipeline error: {e}")
        failed += 1

    # =========================================================================
    # Summary
    # =========================================================================
    print("\n" + "=" * 70)
    print(f"Results: {passed} passed, {failed} failed")
    print("=" * 70)

    print("""
Tests covered:
  - All JIT modes: int, float, int32, float32, complex128, complex64, bool, object
  - Mode chains: int->float, float->int32, int32->float32
  - Factorial computation
  - LLVM IR generation
  - inline_c: C functions, C->JIT, JIT->C chains
  - ptr mode: array access, ptr->JIT, ptr->C->JIT chains
  - GIL/RAII: acquire/release, parallel work, type conversion, refcount
  - Generators: countdown, sum
  - Grand pipeline: int->C->float->float32->complex
""")

    if failed > 0:
        print("CI TEST FAILED")
        sys.exit(1)
    else:
        print("CI TEST PASSED")
        sys.exit(0)


if __name__ == "__main__":
    main()
