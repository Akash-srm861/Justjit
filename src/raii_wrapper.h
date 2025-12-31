/**
 * raii_wrapper.h - World-Class RAII Wrappers for Python-C Bidirectional Interop
 * 
 * Provides:
 * - ScopeGuard: Generic cleanup on scope exit
 * - GILGuard/GILRelease: Python GIL management
 * - PyObjectPtr: Python object lifetime management
 * - NumpyBuffer: Zero-copy NumPy array access
 * - Type converters: Python <-> C type conversion
 */

#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <utility>
#include <functional>
#include <type_traits>

namespace justjit {

// ============================================================================
// ScopeGuard - Execute cleanup on scope exit (like Rust's Drop)
// ============================================================================
template<typename F>
class ScopeGuard {
public:
    explicit ScopeGuard(F cleanup) noexcept 
        : cleanup_(std::move(cleanup)), active_(true) {}
    
    ~ScopeGuard() {
        if (active_) {
            try { cleanup_(); } catch (...) {}
        }
    }
    
    // Move only
    ScopeGuard(ScopeGuard&& other) noexcept
        : cleanup_(std::move(other.cleanup_)), active_(other.active_) {
        other.active_ = false;
    }
    
    // Dismiss - don't run cleanup
    void dismiss() noexcept { active_ = false; }
    
    // Non-copyable
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard& operator=(ScopeGuard&&) = delete;

private:
    F cleanup_;
    bool active_;
};

template<typename F>
[[nodiscard]] ScopeGuard<F> make_guard(F cleanup) {
    return ScopeGuard<F>(std::move(cleanup));
}

// ============================================================================
// GILGuard - Acquire GIL on construction, release on destruction
// ============================================================================
class GILGuard {
public:
    GILGuard() : state_(PyGILState_Ensure()) {}
    ~GILGuard() { PyGILState_Release(state_); }
    
    GILGuard(const GILGuard&) = delete;
    GILGuard& operator=(const GILGuard&) = delete;
    GILGuard(GILGuard&&) = delete;
    GILGuard& operator=(GILGuard&&) = delete;

private:
    PyGILState_STATE state_;
};

// ============================================================================
// GILRelease - Release GIL for parallel C code, reacquire on destruction
// ============================================================================
class GILRelease {
public:
    GILRelease() : save_(PyEval_SaveThread()) {}
    ~GILRelease() { PyEval_RestoreThread(save_); }
    
    GILRelease(const GILRelease&) = delete;
    GILRelease& operator=(const GILRelease&) = delete;
    GILRelease(GILRelease&&) = delete;
    GILRelease& operator=(GILRelease&&) = delete;

private:
    PyThreadState* save_;
};

// ============================================================================
// PyObjectPtr - RAII wrapper for PyObject* with proper ref counting
// ============================================================================
class PyObjectPtr {
public:
    PyObjectPtr() noexcept : ptr_(nullptr) {}
    explicit PyObjectPtr(PyObject* p) noexcept : ptr_(p) {}
    
    // Steal reference (take ownership without INCREF)
    static PyObjectPtr steal(PyObject* p) noexcept {
        return PyObjectPtr(p);
    }
    
    // Borrow reference (INCREF on construction)
    static PyObjectPtr borrow(PyObject* p) noexcept {
        Py_XINCREF(p);
        return PyObjectPtr(p);
    }
    
    ~PyObjectPtr() { Py_XDECREF(ptr_); }
    
    // Move semantics
    PyObjectPtr(PyObjectPtr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }
    
    PyObjectPtr& operator=(PyObjectPtr&& other) noexcept {
        if (this != &other) {
            Py_XDECREF(ptr_);
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }
    
    // Non-copyable
    PyObjectPtr(const PyObjectPtr&) = delete;
    PyObjectPtr& operator=(const PyObjectPtr&) = delete;
    
    // Access
    PyObject* get() const noexcept { return ptr_; }
    PyObject* operator->() const noexcept { return ptr_; }
    PyObject& operator*() const noexcept { return *ptr_; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }
    
    // Release ownership
    PyObject* release() noexcept {
        PyObject* p = ptr_;
        ptr_ = nullptr;
        return p;
    }
    
    // Reset with new pointer
    void reset(PyObject* p = nullptr) noexcept {
        Py_XDECREF(ptr_);
        ptr_ = p;
    }

private:
    PyObject* ptr_;
};

// ============================================================================
// NumpyBuffer - Zero-copy access to NumPy array data via buffer protocol
// ============================================================================
class NumpyBuffer {
public:
    NumpyBuffer() noexcept : valid_(false) {
        view_.obj = nullptr;
    }
    
    explicit NumpyBuffer(PyObject* arr) noexcept : valid_(false) {
        view_.obj = nullptr;
        if (PyObject_GetBuffer(arr, &view_, PyBUF_STRIDES | PyBUF_FORMAT) == 0) {
            valid_ = true;
        }
    }
    
    ~NumpyBuffer() {
        if (valid_) {
            PyBuffer_Release(&view_);
        }
    }
    
    // Move semantics
    NumpyBuffer(NumpyBuffer&& other) noexcept 
        : view_(other.view_), valid_(other.valid_) {
        other.valid_ = false;
        other.view_.obj = nullptr;
    }
    
    NumpyBuffer& operator=(NumpyBuffer&& other) noexcept {
        if (this != &other) {
            if (valid_) PyBuffer_Release(&view_);
            view_ = other.view_;
            valid_ = other.valid_;
            other.valid_ = false;
            other.view_.obj = nullptr;
        }
        return *this;
    }
    
    // Non-copyable
    NumpyBuffer(const NumpyBuffer&) = delete;
    NumpyBuffer& operator=(const NumpyBuffer&) = delete;
    
    // Access
    bool valid() const noexcept { return valid_; }
    void* data() const noexcept { return view_.buf; }
    Py_ssize_t size() const noexcept { return view_.len; }
    Py_ssize_t itemsize() const noexcept { return view_.itemsize; }
    Py_ssize_t ndim() const noexcept { return view_.ndim; }
    Py_ssize_t* shape() const noexcept { return view_.shape; }
    Py_ssize_t* strides() const noexcept { return view_.strides; }
    const char* format() const noexcept { return view_.format; }
    bool readonly() const noexcept { return view_.readonly != 0; }
    
    // Typed access
    template<typename T>
    T* as() const noexcept { return static_cast<T*>(view_.buf); }

private:
    Py_buffer view_;
    bool valid_;
};

// ============================================================================
// Type Converters - Python <-> C bidirectional conversion
// ============================================================================

// Python -> C
inline long long py_to_long(PyObject* obj) {
    return PyLong_AsLongLong(obj);
}

inline double py_to_double(PyObject* obj) {
    if (PyFloat_Check(obj)) {
        return PyFloat_AsDouble(obj);
    } else if (PyLong_Check(obj)) {
        return static_cast<double>(PyLong_AsLongLong(obj));
    }
    return 0.0;
}

inline const char* py_to_string(PyObject* obj) {
    return PyUnicode_AsUTF8(obj);
}

inline bool py_to_bool(PyObject* obj) {
    return PyObject_IsTrue(obj) != 0;
}

// C -> Python (returns new reference)
inline PyObject* long_to_py(long long val) {
    return PyLong_FromLongLong(val);
}

inline PyObject* double_to_py(double val) {
    return PyFloat_FromDouble(val);
}

inline PyObject* string_to_py(const char* val) {
    return PyUnicode_FromString(val);
}

inline PyObject* bool_to_py(bool val) {
    return PyBool_FromLong(val ? 1 : 0);
}

// ============================================================================
// C API exports - Registered as JIT symbols for inline C code
// ============================================================================

#if defined(_WIN32)
  #define JIT_EXPORT __declspec(dllexport)
#else
  #define JIT_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    // GIL management
    JIT_EXPORT void* jit_gil_acquire();
    JIT_EXPORT void jit_gil_release(void* guard);
    JIT_EXPORT void* jit_gil_release_begin();
    JIT_EXPORT void jit_gil_release_end(void* save);
    
    // Python object management
    JIT_EXPORT void* jit_pyobj_new(PyObject* p);
    JIT_EXPORT void jit_pyobj_free(void* ptr);
    JIT_EXPORT PyObject* jit_pyobj_get(void* ptr);
    
    // Buffer access
    JIT_EXPORT void* jit_buffer_new(PyObject* arr);
    JIT_EXPORT void jit_buffer_free(void* buf);
    JIT_EXPORT void* jit_buffer_data(void* buf);
    JIT_EXPORT Py_ssize_t jit_buffer_size(void* buf);
    
    // Type conversions
    JIT_EXPORT long long jit_py_to_long(PyObject* obj);
    JIT_EXPORT double jit_py_to_double(PyObject* obj);
    JIT_EXPORT const char* jit_py_to_string(PyObject* obj);
    JIT_EXPORT PyObject* jit_long_to_py(long long val);
    JIT_EXPORT PyObject* jit_double_to_py(double val);
    JIT_EXPORT PyObject* jit_string_to_py(const char* val);
    
    // Python function call from C
    JIT_EXPORT PyObject* jit_call_python(PyObject* func, PyObject* args);
    
    // List operations
    JIT_EXPORT PyObject* jit_list_new(Py_ssize_t size);
    JIT_EXPORT Py_ssize_t jit_list_size(PyObject* list);
    JIT_EXPORT PyObject* jit_list_get(PyObject* list, Py_ssize_t index);
    JIT_EXPORT int jit_list_set(PyObject* list, Py_ssize_t index, PyObject* item);
    JIT_EXPORT int jit_list_append(PyObject* list, PyObject* item);
    
    // Dict operations
    JIT_EXPORT PyObject* jit_dict_new();
    JIT_EXPORT PyObject* jit_dict_get(PyObject* dict, const char* key);
    JIT_EXPORT PyObject* jit_dict_get_obj(PyObject* dict, PyObject* key);
    JIT_EXPORT int jit_dict_set(PyObject* dict, const char* key, PyObject* val);
    JIT_EXPORT int jit_dict_set_obj(PyObject* dict, PyObject* key, PyObject* val);
    JIT_EXPORT int jit_dict_del(PyObject* dict, const char* key);
    JIT_EXPORT PyObject* jit_dict_keys(PyObject* dict);
    
    // Tuple operations
    JIT_EXPORT PyObject* jit_tuple_new(Py_ssize_t size);
    JIT_EXPORT PyObject* jit_tuple_get(PyObject* tuple, Py_ssize_t index);
    JIT_EXPORT int jit_tuple_set(PyObject* tuple, Py_ssize_t index, PyObject* item);
    
    // Object attribute/method access
    JIT_EXPORT PyObject* jit_getattr(PyObject* obj, const char* name);
    JIT_EXPORT int jit_setattr(PyObject* obj, const char* name, PyObject* val);
    JIT_EXPORT int jit_hasattr(PyObject* obj, const char* name);
    JIT_EXPORT PyObject* jit_call_method(PyObject* obj, const char* method, PyObject* args);
    JIT_EXPORT PyObject* jit_call_method0(PyObject* obj, const char* method);
    
    // Reference counting
    JIT_EXPORT void jit_incref(PyObject* obj);
    JIT_EXPORT void jit_decref(PyObject* obj);
    
    // Module import
    JIT_EXPORT PyObject* jit_import(const char* name);
    
    // Sequence/iterator operations
    JIT_EXPORT Py_ssize_t jit_len(PyObject* obj);
    JIT_EXPORT PyObject* jit_getitem(PyObject* obj, Py_ssize_t index);
    JIT_EXPORT int jit_setitem(PyObject* obj, Py_ssize_t index, PyObject* val);
    JIT_EXPORT PyObject* jit_getitem_obj(PyObject* obj, PyObject* key);
    JIT_EXPORT int jit_setitem_obj(PyObject* obj, PyObject* key, PyObject* val);
    
    // Type checking
    JIT_EXPORT int jit_is_list(PyObject* obj);
    JIT_EXPORT int jit_is_dict(PyObject* obj);
    JIT_EXPORT int jit_is_tuple(PyObject* obj);
    JIT_EXPORT int jit_is_int(PyObject* obj);
    JIT_EXPORT int jit_is_float(PyObject* obj);
    JIT_EXPORT int jit_is_str(PyObject* obj);
    JIT_EXPORT int jit_is_none(PyObject* obj);
    JIT_EXPORT int jit_is_callable(PyObject* obj);
    
    // Constants
    JIT_EXPORT PyObject* jit_none();
    JIT_EXPORT PyObject* jit_true();
    JIT_EXPORT PyObject* jit_false();
    
// Error handling
JIT_EXPORT int jit_error_occurred();
JIT_EXPORT void jit_error_clear();
JIT_EXPORT void jit_error_print();

// =========================================================================
// Enhanced Callback Functions for Bidirectional Interop
// =========================================================================

// Call Python function with 1 argument
JIT_EXPORT PyObject* jit_call1(PyObject* func, PyObject* arg);

// Call Python function with 2 arguments
JIT_EXPORT PyObject* jit_call2(PyObject* func, PyObject* arg1, PyObject* arg2);

// Call Python function with 3 arguments
JIT_EXPORT PyObject* jit_call3(PyObject* func, PyObject* arg1, PyObject* arg2, PyObject* arg3);

// Call method with 1 argument
JIT_EXPORT PyObject* jit_call_method1(PyObject* obj, const char* method, PyObject* arg);

// Call method with 2 arguments
JIT_EXPORT PyObject* jit_call_method2(PyObject* obj, const char* method, PyObject* arg1, PyObject* arg2);

// Build tuple from arguments
JIT_EXPORT PyObject* jit_build_args1(PyObject* arg);
JIT_EXPORT PyObject* jit_build_args2(PyObject* arg1, PyObject* arg2);
JIT_EXPORT PyObject* jit_build_args3(PyObject* arg1, PyObject* arg2, PyObject* arg3);

// Convert C types to Python and build args
JIT_EXPORT PyObject* jit_build_int_args1(long long v1);
JIT_EXPORT PyObject* jit_build_int_args2(long long v1, long long v2);
JIT_EXPORT PyObject* jit_build_float_args1(double v1);
JIT_EXPORT PyObject* jit_build_float_args2(double v1, double v2);

// Iterator support
JIT_EXPORT PyObject* jit_get_iter(PyObject* obj);
JIT_EXPORT PyObject* jit_iter_next(PyObject* iter);
JIT_EXPORT int jit_iter_check(PyObject* obj);


// Bytes/bytearray support
JIT_EXPORT PyObject* jit_bytes_new(const char* data, Py_ssize_t len);
JIT_EXPORT const char* jit_bytes_data(PyObject* bytes);
JIT_EXPORT Py_ssize_t jit_bytes_len(PyObject* bytes);

// Simplified Python Expression Evaluation
JIT_EXPORT PyObject* jit_py_eval(const char* expr, PyObject* locals);
JIT_EXPORT PyObject* jit_py_exec(const char* code, PyObject* locals);

} // extern "C"

} // namespace justjit


