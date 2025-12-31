RAII Wrappers and C API
========================

JustJIT provides a comprehensive C API for use within ``inline_c`` code. These functions enable safe Python-C interop, GIL management, and zero-copy buffer access.

GIL Management
--------------

When calling Python from C code, you must hold the Global Interpreter Lock (GIL). When doing compute-intensive C work, releasing the GIL allows Python threads to run.

**Acquire/Release Cycle:**

.. code-block:: c

   void* guard = jit_gil_acquire();
   // ... call Python APIs safely ...
   jit_gil_release(guard);

**Release for Parallel Work:**

.. code-block:: c

   void* save = jit_gil_release_begin();
   // ... compute-intensive work without GIL ...
   // Python threads can run during this time
   jit_gil_release_end(save);

**Functions:**

.. c:function:: void* jit_gil_acquire(void)

   Acquire the Python GIL. Returns a handle that must be passed to ``jit_gil_release``.

.. c:function:: void jit_gil_release(void* guard)

   Release a GIL acquired with ``jit_gil_acquire``.

.. c:function:: void* jit_gil_release_begin(void)

   Release the GIL to allow parallel execution. Returns a handle for ``jit_gil_release_end``.

.. c:function:: void jit_gil_release_end(void* save)

   Reacquire the GIL after parallel work.

Type Conversion
---------------

Convert between Python objects and C types:

**Python to C:**

.. code-block:: c

   long long val = jit_py_to_long(py_obj);
   double d = jit_py_to_double(py_obj);
   const char* s = jit_py_to_string(py_obj);

**C to Python:**

.. code-block:: c

   PyObject* py_int = jit_long_to_py(42);
   PyObject* py_float = jit_double_to_py(3.14);
   PyObject* py_str = jit_string_to_py("hello");
   // Remember to call jit_decref when done

**Functions:**

.. c:function:: long long jit_py_to_long(PyObject* obj)

   Convert Python int to C long long.

.. c:function:: double jit_py_to_double(PyObject* obj)

   Convert Python int or float to C double.

.. c:function:: const char* jit_py_to_string(PyObject* obj)

   Get UTF-8 string from Python str. Do not free the returned pointer.

.. c:function:: PyObject* jit_long_to_py(long long val)

   Create Python int from C long long. Returns new reference.

.. c:function:: PyObject* jit_double_to_py(double val)

   Create Python float from C double. Returns new reference.

.. c:function:: PyObject* jit_string_to_py(const char* val)

   Create Python str from C string. Returns new reference.

Buffer Access
-------------

Access NumPy array data directly without copying:

.. code-block:: c

   void* buf = jit_buffer_new(numpy_array);
   if (buf) {
       double* data = (double*)jit_buffer_data(buf);
       long long size = jit_buffer_size(buf);
       
       for (long long i = 0; i < size; i++) {
           data[i] *= 2.0;  // Modify in place
       }
       
       jit_buffer_free(buf);  // Release buffer
   }

**Functions:**

.. c:function:: void* jit_buffer_new(PyObject* arr)

   Create buffer handle from NumPy array. Returns NULL if invalid.

.. c:function:: void jit_buffer_free(void* buf)

   Release buffer handle.

.. c:function:: void* jit_buffer_data(void* buf)

   Get raw data pointer from buffer.

.. c:function:: Py_ssize_t jit_buffer_size(void* buf)

   Get buffer size in bytes.

Reference Counting
------------------

Manage Python object lifetimes from C:

.. code-block:: c

   PyObject* obj = jit_long_to_py(42);
   jit_incref(obj);   // Add reference
   // ... use obj ...
   jit_decref(obj);   // Remove added reference
   jit_decref(obj);   // Remove original reference

**Functions:**

.. c:function:: void jit_incref(PyObject* obj)

   Increment reference count. Safe with NULL.

.. c:function:: void jit_decref(PyObject* obj)

   Decrement reference count. Safe with NULL.

Calling Python Functions
------------------------

Call Python functions from C:

.. code-block:: c

   // Call with 1 argument
   PyObject* result = jit_call1(py_func, arg1);
   
   // Call with 2 arguments
   PyObject* result = jit_call2(py_func, arg1, arg2);
   
   // Call method
   PyObject* result = jit_call_method1(obj, "method_name", arg);

**Functions:**

.. c:function:: PyObject* jit_call1(PyObject* func, PyObject* arg)

   Call Python callable with 1 argument. Returns new reference or NULL on error.

.. c:function:: PyObject* jit_call2(PyObject* func, PyObject* arg1, PyObject* arg2)

   Call Python callable with 2 arguments.

.. c:function:: PyObject* jit_call3(PyObject* func, PyObject* arg1, PyObject* arg2, PyObject* arg3)

   Call Python callable with 3 arguments.

.. c:function:: PyObject* jit_call_method1(PyObject* obj, const char* method, PyObject* arg)

   Call method on object with 1 argument.

Collection Operations
---------------------

Work with Python lists, dicts, and tuples:

**Lists:**

.. code-block:: c

   PyObject* list = jit_list_new(5);
   jit_list_set(list, 0, jit_long_to_py(42));
   jit_list_append(list, jit_long_to_py(100));
   PyObject* item = jit_list_get(list, 0);

**Dicts:**

.. code-block:: c

   PyObject* dict = jit_dict_new();
   jit_dict_set(dict, "key", jit_long_to_py(42));
   PyObject* val = jit_dict_get(dict, "key");

**Tuples:**

.. code-block:: c

   PyObject* tuple = jit_tuple_new(2);
   jit_tuple_set(tuple, 0, arg1);
   jit_tuple_set(tuple, 1, arg2);

Type Checking
-------------

Check Python object types:

.. code-block:: c

   if (jit_is_int(obj)) { /* ... */ }
   if (jit_is_float(obj)) { /* ... */ }
   if (jit_is_str(obj)) { /* ... */ }
   if (jit_is_list(obj)) { /* ... */ }
   if (jit_is_dict(obj)) { /* ... */ }
   if (jit_is_none(obj)) { /* ... */ }
   if (jit_is_callable(obj)) { /* ... */ }

Constants
---------

Access Python singletons:

.. code-block:: c

   PyObject* none = jit_none();   // Py_None
   PyObject* t = jit_true();      // Py_True
   PyObject* f = jit_false();     // Py_False

Error Handling
--------------

Check and clear Python errors:

.. code-block:: c

   PyObject* result = jit_call1(func, arg);
   if (jit_error_occurred()) {
       jit_error_print();
       jit_error_clear();
   }

Complete Example
----------------

A complete example using GIL, buffers, and callbacks:

.. code-block:: c

   double process_array(void* py_array, void* py_callback) {
       // Access NumPy array
       void* buf = jit_buffer_new(py_array);
       if (!buf) return -1.0;
       
       double* data = (double*)jit_buffer_data(buf);
       long long size = jit_buffer_size(buf) / sizeof(double);
       
       // Release GIL for parallel work
       void* save = jit_gil_release_begin();
       
       double sum = 0.0;
       for (long long i = 0; i < size; i++) {
           sum += data[i];
       }
       
       // Reacquire GIL before Python callback
       jit_gil_release_end(save);
       
       // Call Python callback with result
       PyObject* py_sum = jit_double_to_py(sum);
       PyObject* result = jit_call1(py_callback, py_sum);
       jit_decref(py_sum);
       
       double final = jit_py_to_double(result);
       jit_decref(result);
       
       jit_buffer_free(buf);
       return final;
   }
