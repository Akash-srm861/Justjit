"""
JustJIT Comprehensive Benchmark Suite

Tests:
- Basic operations
- Edge cases (zero, negative, large numbers, overflow)
- Control flow (branches, loops)
- Correctness verification

Modes:
- CPython baseline
- JIT Object Mode (full Python compatibility)
- JIT Integer Mode (native int64)
"""

import justjit
import time
import sys


# ============================================================================
# Test Suite - CPython Baseline
# ============================================================================

def add_py(a, b):
    return a + b

def subtract_py(a, b):
    return a - b

def multiply_py(a, b):
    return a * b

def divide_py(a, b):
    if b == 0:
        return 0
    return a // b

def modulo_py(a, b):
    if b == 0:
        return 0
    return a % b

def power_py(base, exp):
    result = 1
    i = 0
    while i < exp:
        result = result * base
        i = i + 1
    return result

def sum_while_py(n):
    total = 0
    i = 0
    while i < n:
        total = total + i
        i = i + 1
    return total

def fibonacci_py(n):
    if n < 2:
        return n
    a = 0
    b = 1
    i = 2
    while i <= n:
        c = a + b
        a = b
        b = c
        i = i + 1
    return b

def count_down_py(n):
    total = 0
    while n > 0:
        total = total + n
        n = n - 1
    return total

def nested_loop_py(n):
    total = 0
    i = 0
    while i < n:
        j = 0
        while j < n:
            total = total + 1
            j = j + 1
        i = i + 1
    return total

def branch_heavy_py(n):
    total = 0
    i = 0
    while i < n:
        if i % 2 == 0:
            total = total + i
        else:
            total = total - 1
        i = i + 1
    return total

def abs_py(x):
    if x < 0:
        return -x
    return x

def max_py(a, b):
    if a > b:
        return a
    return b

def min_py(a, b):
    if a < b:
        return a
    return b

def clamp_py(x, lo, hi):
    if x < lo:
        return lo
    if x > hi:
        return hi
    return x

def gcd_py(a, b):
    if a < 0:
        a = -a
    if b < 0:
        b = -b
    while b != 0:
        t = b
        b = a % b
        a = t
    return a


# ============================================================================
# Test Suite - JIT Object Mode
# ============================================================================

@justjit.jit(mode="object")
def add_obj(a, b):
    return a + b

@justjit.jit(mode="object")
def subtract_obj(a, b):
    return a - b

@justjit.jit(mode="object")
def multiply_obj(a, b):
    return a * b

@justjit.jit(mode="object")
def divide_obj(a, b):
    if b == 0:
        return 0
    return a // b

@justjit.jit(mode="object")
def modulo_obj(a, b):
    if b == 0:
        return 0
    return a % b

@justjit.jit(mode="object")
def power_obj(base, exp):
    result = 1
    i = 0
    while i < exp:
        result = result * base
        i = i + 1
    return result

@justjit.jit(mode="object")
def sum_while_obj(n):
    total = 0
    i = 0
    while i < n:
        total = total + i
        i = i + 1
    return total

@justjit.jit(mode="object")
def fibonacci_obj(n):
    if n < 2:
        return n
    a = 0
    b = 1
    i = 2
    while i <= n:
        c = a + b
        a = b
        b = c
        i = i + 1
    return b

@justjit.jit(mode="object")
def count_down_obj(n):
    total = 0
    while n > 0:
        total = total + n
        n = n - 1
    return total

@justjit.jit(mode="object")
def nested_loop_obj(n):
    total = 0
    i = 0
    while i < n:
        j = 0
        while j < n:
            total = total + 1
            j = j + 1
        i = i + 1
    return total

@justjit.jit(mode="object")
def branch_heavy_obj(n):
    total = 0
    i = 0
    while i < n:
        if i % 2 == 0:
            total = total + i
        else:
            total = total - 1
        i = i + 1
    return total

@justjit.jit(mode="object")
def abs_obj(x):
    if x < 0:
        return -x
    return x

@justjit.jit(mode="object")
def max_obj(a, b):
    if a > b:
        return a
    return b

@justjit.jit(mode="object")
def min_obj(a, b):
    if a < b:
        return a
    return b

@justjit.jit(mode="object")
def clamp_obj(x, lo, hi):
    if x < lo:
        return lo
    if x > hi:
        return hi
    return x

@justjit.jit(mode="object")
def gcd_obj(a, b):
    if a < 0:
        a = -a
    if b < 0:
        b = -b
    while b != 0:
        t = b
        b = a % b
        a = t
    return a


# ============================================================================
# Test Suite - JIT Integer Mode
# ============================================================================

@justjit.jit(mode="int")
def add_int(a, b):
    return a + b

@justjit.jit(mode="int")
def subtract_int(a, b):
    return a - b

@justjit.jit(mode="int")
def multiply_int(a, b):
    return a * b

@justjit.jit(mode="int")
def divide_int(a, b):
    if b == 0:
        return 0
    return a // b

@justjit.jit(mode="int")
def modulo_int(a, b):
    if b == 0:
        return 0
    return a % b

@justjit.jit(mode="int")
def power_int(base, exp):
    result = 1
    i = 0
    while i < exp:
        result = result * base
        i = i + 1
    return result

@justjit.jit(mode="int")
def sum_while_int(n):
    total = 0
    i = 0
    while i < n:
        total = total + i
        i = i + 1
    return total

@justjit.jit(mode="int")
def fibonacci_int(n):
    if n < 2:
        return n
    a = 0
    b = 1
    i = 2
    while i <= n:
        c = a + b
        a = b
        b = c
        i = i + 1
    return b

@justjit.jit(mode="int")
def count_down_int(n):
    total = 0
    while n > 0:
        total = total + n
        n = n - 1
    return total

@justjit.jit(mode="int")
def nested_loop_int(n):
    total = 0
    i = 0
    while i < n:
        j = 0
        while j < n:
            total = total + 1
            j = j + 1
        i = i + 1
    return total

@justjit.jit(mode="int")
def branch_heavy_int(n):
    total = 0
    i = 0
    while i < n:
        if i % 2 == 0:
            total = total + i
        else:
            total = total - 1
        i = i + 1
    return total

@justjit.jit(mode="int")
def abs_int(x):
    if x < 0:
        return -x
    return x

@justjit.jit(mode="int")
def max_int(a, b):
    if a > b:
        return a
    return b

@justjit.jit(mode="int")
def min_int(a, b):
    if a < b:
        return a
    return b

@justjit.jit(mode="int")
def clamp_int(x, lo, hi):
    if x < lo:
        return lo
    if x > hi:
        return hi
    return x

@justjit.jit(mode="int")
def gcd_int(a, b):
    if a < 0:
        a = -a
    if b < 0:
        b = -b
    while b != 0:
        t = b
        b = a % b
        a = t
    return a


# ============================================================================
# Utilities
# ============================================================================

def benchmark(func, *args, warmup=3, iterations=10):
    for _ in range(warmup):
        try:
            func(*args)
        except:
            return -1, None
    
    times = []
    for _ in range(iterations):
        start = time.perf_counter()
        try:
            result = func(*args)
        except:
            return -1, None
        end = time.perf_counter()
        times.append((end - start) * 1000)
    
    times.sort()
    return times[len(times) // 2], result


def verify(name, py_func, obj_func, int_func, *args):
    """Verify all three modes produce same result."""
    py_result = py_func(*args)
    obj_result = obj_func(*args)
    int_result = int_func(*args)
    
    if py_result == obj_result == int_result:
        return True, py_result
    else:
        print(f"  FAIL: {name}{args} -> py={py_result}, obj={obj_result}, int={int_result}")
        return False, None


def format_time(ms):
    if ms < 0:
        return "   N/A"
    if ms < 0.001:
        return f"{ms*1000:>6.2f}us"
    if ms < 1:
        return f"{ms*1000:>6.1f}us"
    return f"{ms:>6.2f}ms"


def format_speedup(baseline, other):
    if other <= 0 or baseline <= 0:
        return "   N/A"
    ratio = baseline / other
    if ratio >= 100:
        return f"{ratio:>5.0f}x"
    return f"{ratio:>5.1f}x"


# ============================================================================
# Test Cases
# ============================================================================

def run_correctness_tests():
    """Run edge case correctness tests."""
    print("\n" + "=" * 70)
    print("Correctness Tests (Edge Cases)")
    print("=" * 70)
    
    tests = [
        # Basic operations
        ("add", add_py, add_obj, add_int, 0, 0),
        ("add", add_py, add_obj, add_int, 1, -1),
        ("add", add_py, add_obj, add_int, -100, 100),
        ("add", add_py, add_obj, add_int, 2**30, 2**30),
        
        ("subtract", subtract_py, subtract_obj, subtract_int, 0, 0),
        ("subtract", subtract_py, subtract_obj, subtract_int, 10, 20),
        ("subtract", subtract_py, subtract_obj, subtract_int, -5, -10),
        
        ("multiply", multiply_py, multiply_obj, multiply_int, 0, 1000),
        ("multiply", multiply_py, multiply_obj, multiply_int, -7, 8),
        ("multiply", multiply_py, multiply_obj, multiply_int, 12345, 67890),
        
        ("divide", divide_py, divide_obj, divide_int, 100, 7),
        ("divide", divide_py, divide_obj, divide_int, -100, 7),
        ("divide", divide_py, divide_obj, divide_int, 100, 0),  # div by zero
        
        ("modulo", modulo_py, modulo_obj, modulo_int, 100, 7),
        ("modulo", modulo_py, modulo_obj, modulo_int, 0, 5),
        ("modulo", modulo_py, modulo_obj, modulo_int, 100, 0),  # mod by zero
        
        # Power
        ("power", power_py, power_obj, power_int, 2, 0),
        ("power", power_py, power_obj, power_int, 2, 10),
        ("power", power_py, power_obj, power_int, 3, 5),
        
        # Fibonacci edge cases
        ("fibonacci", fibonacci_py, fibonacci_obj, fibonacci_int, 0),
        ("fibonacci", fibonacci_py, fibonacci_obj, fibonacci_int, 1),
        ("fibonacci", fibonacci_py, fibonacci_obj, fibonacci_int, 2),
        ("fibonacci", fibonacci_py, fibonacci_obj, fibonacci_int, 20),
        
        # Sum edge cases
        ("sum_while", sum_while_py, sum_while_obj, sum_while_int, 0),
        ("sum_while", sum_while_py, sum_while_obj, sum_while_int, 1),
        ("sum_while", sum_while_py, sum_while_obj, sum_while_int, 100),
        
        # Count down
        ("count_down", count_down_py, count_down_obj, count_down_int, 0),
        ("count_down", count_down_py, count_down_obj, count_down_int, 10),
        
        # Nested loops
        ("nested_loop", nested_loop_py, nested_loop_obj, nested_loop_int, 0),
        ("nested_loop", nested_loop_py, nested_loop_obj, nested_loop_int, 5),
        
        # Branch heavy
        ("branch_heavy", branch_heavy_py, branch_heavy_obj, branch_heavy_int, 0),
        ("branch_heavy", branch_heavy_py, branch_heavy_obj, branch_heavy_int, 10),
        
        # Utility functions
        ("abs", abs_py, abs_obj, abs_int, 0),
        ("abs", abs_py, abs_obj, abs_int, 42),
        ("abs", abs_py, abs_obj, abs_int, -42),
        
        ("max", max_py, max_obj, max_int, 5, 10),
        ("max", max_py, max_obj, max_int, 10, 5),
        ("max", max_py, max_obj, max_int, -5, -10),
        
        ("min", min_py, min_obj, min_int, 5, 10),
        ("min", min_py, min_obj, min_int, 10, 5),
        ("min", min_py, min_obj, min_int, -5, -10),
        
        ("clamp", clamp_py, clamp_obj, clamp_int, 5, 0, 10),
        ("clamp", clamp_py, clamp_obj, clamp_int, -5, 0, 10),
        ("clamp", clamp_py, clamp_obj, clamp_int, 15, 0, 10),
        
        # GCD
        ("gcd", gcd_py, gcd_obj, gcd_int, 48, 18),
        ("gcd", gcd_py, gcd_obj, gcd_int, 0, 5),
        ("gcd", gcd_py, gcd_obj, gcd_int, -12, 8),
    ]
    
    passed = 0
    failed = 0
    
    for test in tests:
        name = test[0]
        py_func, obj_func, int_func = test[1:4]
        args = test[4:]
        
        ok, result = verify(name, py_func, obj_func, int_func, *args)
        if ok:
            passed += 1
        else:
            failed += 1
    
    print(f"\nResults: {passed} passed, {failed} failed")
    return failed == 0


def run_performance_tests():
    """Run performance benchmarks."""
    print("\n" + "=" * 70)
    print("Performance Benchmarks")
    print("=" * 70)
    
    benchmarks = [
        ("sum_while(100K)", sum_while_py, sum_while_obj, sum_while_int, 100_000),
        ("fibonacci(35)", fibonacci_py, fibonacci_obj, fibonacci_int, 35),
        ("nested_loop(100)", nested_loop_py, nested_loop_obj, nested_loop_int, 100),
        ("branch_heavy(10K)", branch_heavy_py, branch_heavy_obj, branch_heavy_int, 10_000),
        ("gcd(987654321, 123456789)", gcd_py, gcd_obj, gcd_int, 987654321, 123456789),
        ("power(2, 30)", power_py, power_obj, power_int, 2, 30),
    ]
    
    print(f"\n{'Benchmark':<28} {'CPython':>10} {'Object':>10} {'Int':>10} {'Obj Spd':>8} {'Int Spd':>8}")
    print("-" * 80)
    
    for test in benchmarks:
        name = test[0]
        py_func, obj_func, int_func = test[1:4]
        args = test[4:]
        
        py_time, _ = benchmark(py_func, *args)
        obj_time, _ = benchmark(obj_func, *args)
        int_time, _ = benchmark(int_func, *args)
        
        print(f"{name:<28} {format_time(py_time):>10} {format_time(obj_time):>10} {format_time(int_time):>10} "
              f"{format_speedup(py_time, obj_time):>8} {format_speedup(py_time, int_time):>8}")


def run_ir_dump():
    """Dump LLVM IR for multiple functions and save to file."""
    output_file = "justjit_ir_dump.ll"
    
    # Functions to dump IR for
    functions_to_dump = [
        ("add_obj", add_obj, (1, 2)),
        ("subtract_obj", subtract_obj, (10, 5)),
        ("multiply_obj", multiply_obj, (3, 4)),
        ("divide_obj", divide_obj, (10, 3)),
        ("sum_while_obj", sum_while_obj, (10,)),
        ("fibonacci_obj", fibonacci_obj, (10,)),
        ("nested_loop_obj", nested_loop_obj, (5,)),
        ("gcd_obj", gcd_obj, (48, 18)),
        ("abs_obj", abs_obj, (-5,)),
        ("max_obj", max_obj, (3, 7)),
        ("min_obj", min_obj, (3, 7)),
        ("clamp_obj", clamp_obj, (5, 0, 10)),
    ]
    
    print(f"\n" + "=" * 70)
    print(f"Saving LLVM IR to: {output_file}")
    print("=" * 70)
    
    with open(output_file, "w") as f:
        f.write("; JustJIT LLVM IR Dump\n")
        f.write(f"; Generated by benchmark.py\n")
        f.write(f"; Python {sys.version.split()[0]} | LLVM 20 | justjit {justjit.__version__}\n")
        f.write(";\n")
        f.write("; " + "=" * 68 + "\n\n")
        
        for name, func, args in functions_to_dump:
            # Trigger compilation
            try:
                func(*args)
            except:
                pass
            
            # Dump IR
            try:
                ir = justjit.dump_ir(func)
                if ir:
                    f.write(f"; {'=' * 68}\n")
                    f.write(f"; Function: {name}\n")
                    f.write(f"; {'=' * 68}\n\n")
                    f.write(ir)
                    f.write("\n\n")
                    print(f"  Dumped: {name}")
            except Exception as e:
                print(f"  Error dumping {name}: {e}")
    
    print(f"\nIR saved to: {output_file}")


# ============================================================================
# Main
# ============================================================================

def main():
    print("=" * 70)
    print("JustJIT Comprehensive Benchmark Suite")
    print(f"Python {sys.version.split()[0]} | LLVM 20 | justjit {justjit.__version__}")
    print("=" * 70)
    
    all_passed = run_correctness_tests()
    run_performance_tests()
    run_ir_dump()
    
    print("\n" + "=" * 70)
    if all_passed:
        print("All tests passed!")
    else:
        print("Some tests failed!")
    print("=" * 70)


if __name__ == "__main__":
    main()
