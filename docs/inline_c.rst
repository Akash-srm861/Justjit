Inline C/C++ Compilation
=========================

JustJIT includes an embedded Clang compiler that allows you to write and execute C/C++ code directly from Python. This is useful for:

- Performance-critical code that needs raw CPU speed
- Calling existing C libraries
- SIMD/vectorized computations
- Low-level memory access

Basic Usage
-----------

The ``inline_c`` function compiles C code and returns a dictionary of callable functions:

.. code-block:: python

   from justjit import inline_c

   result = inline_c('''
       int add(int a, int b) {
           return a + b;
       }
       
       double multiply(double x, double y) {
           return x * y;
       }
   ''')

   # Call the compiled functions
   print(result['add'](3, 5))           # Output: 8
   print(result['multiply'](2.5, 4.0))  # Output: 10.0

The return value is a dictionary where:

- ``'functions'`` contains a list of all exported function names
- Each function name maps to a callable Python object

Supported Types
---------------

Function parameters and return types are automatically detected from the C signature:

.. list-table::
   :header-rows: 1
   :widths: 30 30 40

   * - C Type
     - Python Type
     - Notes
   * - ``int``, ``long``, ``long long``
     - ``int``
     - 64-bit integers
   * - ``float``, ``double``
     - ``float``
     - 64-bit floats (doubles)
   * - ``void*``, ``char*``, pointers
     - ``int`` (address)
     - Pass as raw address
   * - ``void`` (return)
     - ``None``
     - No return value

Mixed parameter types are fully supported:

.. code-block:: python

   result = inline_c('''
       double mixed(int count, double value) {
           return count * value;
       }
   ''')
   
   print(result['mixed'](10, 2.5))  # Output: 25.0

Using Standard Library Headers
------------------------------

You can include standard C library headers:

.. code-block:: python

   result = inline_c('''
       #include <math.h>
       #include <stdio.h>
       
       double compute_sin(double x) {
           return sin(x);
       }
       
       void hello() {
           printf("Hello from C!\\n");
       }
   ''')

Header availability depends on your platform:

- **Windows with MSVC**: Full C/C++ standard library via Visual Studio
- **Windows without dev tools**: Basic C via embedded musl libc
- **Linux**: System glibc or embedded musl
- **macOS**: System SDK or embedded musl

Include Paths
-------------

Add custom include paths for your own headers:

.. code-block:: python

   result = inline_c('''
       #include "myheader.h"
       
       int use_my_func() {
           return my_custom_function();
       }
   ''', include_paths=['/path/to/headers', './local_headers'])

The compiler automatically searches:

1. Current working directory
2. The directory containing the calling Python script
3. User-provided ``include_paths``
4. System/SDK headers (platform-dependent)
5. Embedded musl libc (fallback)

C++ Support
-----------

Use ``lang="c++"`` for C++ code:

.. code-block:: python

   result = inline_c('''
       #include <cmath>
       #include <algorithm>
       
       extern "C" double compute(double x, double y) {
           return std::max(std::sin(x), std::cos(y));
       }
   ''', lang="c++")

   print(result['compute'](3.14, 1.57))

Note: C++ functions must be declared ``extern "C"`` to be callable from Python (prevents name mangling).

Python Interop API
------------------

JustJIT provides a comprehensive API for C code to interact with Python objects:

GIL Management
^^^^^^^^^^^^^^

.. code-block:: c

   // Acquire and release GIL
   void* guard = jit_gil_acquire();
   // ... Python calls here ...
   jit_gil_release(guard);
   
   // Or use macros for scoped release
   JIT_NOGIL_BEGIN;
   // ... CPU-intensive work without GIL ...
   JIT_NOGIL_END;

NumPy Buffer Access
^^^^^^^^^^^^^^^^^^^

.. code-block:: c

   // Get raw pointer to NumPy array data
   JIT_SCOPED_BUFFER(arr, py_array_obj);
   double* data = JIT_SCOPED_BUFFER_DATA(arr, double);
   long long size = JIT_SCOPED_BUFFER_SIZE(arr);
   
   for (long long i = 0; i < size; i++) {
       data[i] *= 2.0;
   }
   // Buffer automatically freed at scope exit

Type Conversions
^^^^^^^^^^^^^^^^

.. code-block:: c

   // Python -> C
   long long i = jit_py_to_long(py_obj);
   double d = jit_py_to_double(py_obj);
   const char* s = jit_py_to_string(py_obj);
   
   // C -> Python
   void* py_int = jit_long_to_py(42);
   void* py_float = jit_double_to_py(3.14);
   void* py_str = jit_string_to_py("hello");

List/Dict/Tuple Operations
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c

   // Lists
   void* list = jit_list_new(10);
   jit_list_append(list, jit_long_to_py(42));
   void* item = jit_list_get(list, 0);
   
   // Dicts
   void* dict = jit_dict_new();
   jit_dict_set(dict, "key", jit_long_to_py(100));
   void* val = jit_dict_get(dict, "key");
   
   // Tuples
   void* tup = jit_tuple_new(3);
   jit_tuple_set(tup, 0, jit_long_to_py(1));

Debugging: Inspecting Generated IR
----------------------------------

Use ``dump_ir=True`` to capture the LLVM IR:

.. code-block:: python

   from justjit import inline_c, dump_c_ir

   result = inline_c('''
       double square(double x) {
           return x * x;
       }
   ''', dump_ir=True)

   # Get the LLVM IR
   ir = dump_c_ir()
   print(ir)

Output:

.. code-block:: llvm

   define double @square(double %x) {
   entry:
     %mul = fmul double %x, %x
     ret double %mul
   }

Error Handling
--------------

Compilation errors are raised as ``RuntimeError``:

.. code-block:: python

   try:
       result = inline_c('''
           int bad_code( {  // Syntax error
               return 1;
           }
       ''')
   except RuntimeError as e:
       print(f"Compilation failed: {e}")

Common errors:

- **Missing headers**: Install development tools or use embedded musl subset
- **Undefined symbols**: Ensure all functions are defined or linked
- **Type mismatches**: Check function signatures match expected types

Platform Notes
--------------

Windows
^^^^^^^

For full C/C++ support, install Visual Studio or Build Tools:

1. Install `Visual Studio Build Tools <https://visualstudio.microsoft.com/downloads/>`_
2. Select "C++ build tools" workload
3. Run Python from "Developer Command Prompt" to set ``%INCLUDE%``

Without dev tools, only basic C (via embedded musl) is available.

Linux
^^^^^

Install development headers:

.. code-block:: bash

   # Ubuntu/Debian
   sudo apt install build-essential
   
   # Fedora/RHEL
   sudo dnf install gcc gcc-c++ glibc-devel

macOS
^^^^^

Install Xcode Command Line Tools:

.. code-block:: bash

   xcode-select --install

Performance Tips
----------------

1. **Release GIL for CPU-bound work**: Use ``JIT_NOGIL_BEGIN/END`` for parallelism
2. **Use raw buffers**: ``JIT_SCOPED_BUFFER`` avoids Python overhead
3. **Batch operations**: Process arrays in C instead of Python loops
4. **Enable SIMD**: Use ``-march=native`` equivalent intrinsics

Example: High-Performance NumPy Operation
-----------------------------------------

.. code-block:: python

   import numpy as np
   from justjit import inline_c

   result = inline_c('''
       void fast_scale(void* arr_obj, double factor) {
           JIT_SCOPED_BUFFER(arr, arr_obj);
           double* data = JIT_SCOPED_BUFFER_DATA(arr, double);
           long long n = JIT_SCOPED_BUFFER_SIZE(arr);
           
           JIT_NOGIL_BEGIN;
           for (long long i = 0; i < n; i++) {
               data[i] *= factor;
           }
           JIT_NOGIL_END;
       }
   ''')

   arr = np.arange(1000000, dtype=np.float64)
   result['fast_scale'](arr, 2.5)

See Also
--------

- :doc:`raii` - Full C API reference for Python interop
- :doc:`api` - Full API reference for ``inline_c``
- :doc:`performance` - Benchmarks and optimization tips
- :doc:`modes` - Other JIT compilation modes

