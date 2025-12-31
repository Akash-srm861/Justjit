/**
 * raii_wrapper.cpp - Implementation of C API exports for bidirectional interop
 * 
 * These functions are registered as JIT symbols via absoluteSymbols() so that
 * compiled inline C code can call Python functions and manage resources.
 */

#include "raii_wrapper.h"

// Use the justjit namespace for C++ classes
using namespace justjit;

// ============================================================================
// C API Exports - Must have C linkage for JIT symbol resolution
// ============================================================================
extern "C" {

// GIL Management - C API for compiled C code
void* jit_gil_acquire() {
    // Allocate and construct GILGuard
    return new GILGuard();
}

void jit_gil_release(void* guard) {
    delete static_cast<GILGuard*>(guard);
}

void* jit_gil_release_begin() {
    // Allocate and construct GILRelease
    return new GILRelease();
}

void jit_gil_release_end(void* save) {
    delete static_cast<GILRelease*>(save);
}

// Python Object Management - C API
void* jit_pyobj_new(PyObject* p) {
    return new PyObjectPtr(PyObjectPtr::steal(p));
}

void jit_pyobj_free(void* ptr) {
    delete static_cast<PyObjectPtr*>(ptr);
}

PyObject* jit_pyobj_get(void* ptr) {
    return static_cast<PyObjectPtr*>(ptr)->get();
}

// Buffer Access - C API for NumPy arrays
void* jit_buffer_new(PyObject* arr) {
    auto* buf = new NumpyBuffer(arr);
    if (!buf->valid()) {
        delete buf;
        return nullptr;
    }
    return buf;
}

void jit_buffer_free(void* buf) {
    delete static_cast<NumpyBuffer*>(buf);
}

void* jit_buffer_data(void* buf) {
    return static_cast<NumpyBuffer*>(buf)->data();
}

Py_ssize_t jit_buffer_size(void* buf) {
    return static_cast<NumpyBuffer*>(buf)->size();
}

// Type Conversions - C API
long long jit_py_to_long(PyObject* obj) {
    return py_to_long(obj);
}

double jit_py_to_double(PyObject* obj) {
    return py_to_double(obj);
}

const char* jit_py_to_string(PyObject* obj) {
    return py_to_string(obj);
}

PyObject* jit_long_to_py(long long val) {
    return long_to_py(val);
}

PyObject* jit_double_to_py(double val) {
    return double_to_py(val);
}

PyObject* jit_string_to_py(const char* val) {
    return string_to_py(val);
}

// Python Function Call from C
PyObject* jit_call_python(PyObject* func, PyObject* args) {
    // GIL must be held by caller
    if (!PyCallable_Check(func)) {
        PyErr_SetString(PyExc_TypeError, "Object is not callable");
        return nullptr;
    }
    return PyObject_Call(func, args, nullptr);
}

// ============================================================================
// List Operations
// ============================================================================
PyObject* jit_list_new(Py_ssize_t size) {
    return PyList_New(size);
}

Py_ssize_t jit_list_size(PyObject* list) {
    if (!PyList_Check(list)) return -1;
    return PyList_Size(list);
}

PyObject* jit_list_get(PyObject* list, Py_ssize_t index) {
    if (!PyList_Check(list)) return nullptr;
    PyObject* item = PyList_GetItem(list, index);  // borrowed reference
    Py_XINCREF(item);  // return new reference for safety
    return item;
}

int jit_list_set(PyObject* list, Py_ssize_t index, PyObject* item) {
    if (!PyList_Check(list)) return -1;
    Py_INCREF(item);  // PyList_SetItem steals reference
    return PyList_SetItem(list, index, item);
}

int jit_list_append(PyObject* list, PyObject* item) {
    if (!PyList_Check(list)) return -1;
    return PyList_Append(list, item);
}

// ============================================================================
// Dict Operations
// ============================================================================
PyObject* jit_dict_new() {
    return PyDict_New();
}

PyObject* jit_dict_get(PyObject* dict, const char* key) {
    if (!PyDict_Check(dict)) return nullptr;
    PyObject* item = PyDict_GetItemString(dict, key);  // borrowed reference
    Py_XINCREF(item);  // return new reference for safety
    return item;
}

PyObject* jit_dict_get_obj(PyObject* dict, PyObject* key) {
    if (!PyDict_Check(dict)) return nullptr;
    PyObject* item = PyDict_GetItem(dict, key);  // borrowed reference
    Py_XINCREF(item);
    return item;
}

int jit_dict_set(PyObject* dict, const char* key, PyObject* val) {
    if (!PyDict_Check(dict)) return -1;
    return PyDict_SetItemString(dict, key, val);
}

int jit_dict_set_obj(PyObject* dict, PyObject* key, PyObject* val) {
    if (!PyDict_Check(dict)) return -1;
    return PyDict_SetItem(dict, key, val);
}

int jit_dict_del(PyObject* dict, const char* key) {
    if (!PyDict_Check(dict)) return -1;
    return PyDict_DelItemString(dict, key);
}

PyObject* jit_dict_keys(PyObject* dict) {
    if (!PyDict_Check(dict)) return nullptr;
    return PyDict_Keys(dict);
}

// ============================================================================
// Tuple Operations
// ============================================================================
PyObject* jit_tuple_new(Py_ssize_t size) {
    return PyTuple_New(size);
}

PyObject* jit_tuple_get(PyObject* tuple, Py_ssize_t index) {
    if (!PyTuple_Check(tuple)) return nullptr;
    PyObject* item = PyTuple_GetItem(tuple, index);  // borrowed reference
    Py_XINCREF(item);
    return item;
}

int jit_tuple_set(PyObject* tuple, Py_ssize_t index, PyObject* item) {
    if (!PyTuple_Check(tuple)) return -1;
    Py_INCREF(item);  // PyTuple_SetItem steals reference
    return PyTuple_SetItem(tuple, index, item);
}

// ============================================================================
// Object Attribute/Method Access
// ============================================================================
PyObject* jit_getattr(PyObject* obj, const char* name) {
    return PyObject_GetAttrString(obj, name);
}

int jit_setattr(PyObject* obj, const char* name, PyObject* val) {
    return PyObject_SetAttrString(obj, name, val);
}

int jit_hasattr(PyObject* obj, const char* name) {
    return PyObject_HasAttrString(obj, name);
}

PyObject* jit_call_method(PyObject* obj, const char* method, PyObject* args) {
    PyObject* meth = PyObject_GetAttrString(obj, method);
    if (!meth) return nullptr;
    PyObject* result = PyObject_Call(meth, args ? args : PyTuple_New(0), nullptr);
    Py_DECREF(meth);
    return result;
}

PyObject* jit_call_method0(PyObject* obj, const char* method) {
    // Call method with no arguments
    PyObject* meth = PyObject_GetAttrString(obj, method);
    if (!meth) return nullptr;
    PyObject* empty_args = PyTuple_New(0);
    PyObject* result = PyObject_Call(meth, empty_args, nullptr);
    Py_DECREF(empty_args);
    Py_DECREF(meth);
    return result;
}

// ============================================================================
// Reference Counting
// ============================================================================
void jit_incref(PyObject* obj) {
    Py_XINCREF(obj);
}

void jit_decref(PyObject* obj) {
    Py_XDECREF(obj);
}

// ============================================================================
// Module Import
// ============================================================================
PyObject* jit_import(const char* name) {
    return PyImport_ImportModule(name);
}

// ============================================================================
// Sequence/Iterator Operations
// ============================================================================
Py_ssize_t jit_len(PyObject* obj) {
    return PyObject_Length(obj);
}

PyObject* jit_getitem(PyObject* obj, Py_ssize_t index) {
    return PySequence_GetItem(obj, index);
}

int jit_setitem(PyObject* obj, Py_ssize_t index, PyObject* val) {
    return PySequence_SetItem(obj, index, val);
}

PyObject* jit_getitem_obj(PyObject* obj, PyObject* key) {
    return PyObject_GetItem(obj, key);
}

int jit_setitem_obj(PyObject* obj, PyObject* key, PyObject* val) {
    return PyObject_SetItem(obj, key, val);
}

// ============================================================================
// Type Checking
// ============================================================================
int jit_is_list(PyObject* obj) { return PyList_Check(obj); }
int jit_is_dict(PyObject* obj) { return PyDict_Check(obj); }
int jit_is_tuple(PyObject* obj) { return PyTuple_Check(obj); }
int jit_is_int(PyObject* obj) { return PyLong_Check(obj); }
int jit_is_float(PyObject* obj) { return PyFloat_Check(obj); }
int jit_is_str(PyObject* obj) { return PyUnicode_Check(obj); }
int jit_is_none(PyObject* obj) { return obj == Py_None; }
int jit_is_callable(PyObject* obj) { return PyCallable_Check(obj); }

// ============================================================================
// Constants
// ============================================================================
PyObject* jit_none() { Py_INCREF(Py_None); return Py_None; }
PyObject* jit_true() { Py_INCREF(Py_True); return Py_True; }
PyObject* jit_false() { Py_INCREF(Py_False); return Py_False; }

// ============================================================================
// Error Handling
// ============================================================================
int jit_error_occurred() { return PyErr_Occurred() != nullptr; }
void jit_error_clear() { PyErr_Clear(); }
void jit_error_print() { PyErr_Print(); }

// ============================================================================
// Enhanced Callback Functions for Bidirectional Interop
// ============================================================================

// Call Python function with 1 argument
PyObject* jit_call1(PyObject* func, PyObject* arg) {
    if (!PyCallable_Check(func)) {
        PyErr_SetString(PyExc_TypeError, "Object is not callable");
        return nullptr;
    }
    PyObject* args = PyTuple_Pack(1, arg);
    if (!args) return nullptr;
    PyObject* result = PyObject_Call(func, args, nullptr);
    Py_DECREF(args);
    return result;
}

// Call Python function with 2 arguments
PyObject* jit_call2(PyObject* func, PyObject* arg1, PyObject* arg2) {
    if (!PyCallable_Check(func)) {
        PyErr_SetString(PyExc_TypeError, "Object is not callable");
        return nullptr;
    }
    PyObject* args = PyTuple_Pack(2, arg1, arg2);
    if (!args) return nullptr;
    PyObject* result = PyObject_Call(func, args, nullptr);
    Py_DECREF(args);
    return result;
}

// Call Python function with 3 arguments
PyObject* jit_call3(PyObject* func, PyObject* arg1, PyObject* arg2, PyObject* arg3) {
    if (!PyCallable_Check(func)) {
        PyErr_SetString(PyExc_TypeError, "Object is not callable");
        return nullptr;
    }
    PyObject* args = PyTuple_Pack(3, arg1, arg2, arg3);
    if (!args) return nullptr;
    PyObject* result = PyObject_Call(func, args, nullptr);
    Py_DECREF(args);
    return result;
}

// Call method with 1 argument
PyObject* jit_call_method1(PyObject* obj, const char* method, PyObject* arg) {
    PyObject* meth = PyObject_GetAttrString(obj, method);
    if (!meth) return nullptr;
    PyObject* args = PyTuple_Pack(1, arg);
    if (!args) { Py_DECREF(meth); return nullptr; }
    PyObject* result = PyObject_Call(meth, args, nullptr);
    Py_DECREF(args);
    Py_DECREF(meth);
    return result;
}

// Call method with 2 arguments
PyObject* jit_call_method2(PyObject* obj, const char* method, PyObject* arg1, PyObject* arg2) {
    PyObject* meth = PyObject_GetAttrString(obj, method);
    if (!meth) return nullptr;
    PyObject* args = PyTuple_Pack(2, arg1, arg2);
    if (!args) { Py_DECREF(meth); return nullptr; }
    PyObject* result = PyObject_Call(meth, args, nullptr);
    Py_DECREF(args);
    Py_DECREF(meth);
    return result;
}

// Build tuple from arguments
PyObject* jit_build_args1(PyObject* arg) {
    return PyTuple_Pack(1, arg);
}

PyObject* jit_build_args2(PyObject* arg1, PyObject* arg2) {
    return PyTuple_Pack(2, arg1, arg2);
}

PyObject* jit_build_args3(PyObject* arg1, PyObject* arg2, PyObject* arg3) {
    return PyTuple_Pack(3, arg1, arg2, arg3);
}

// Convert C types to Python and build args
PyObject* jit_build_int_args1(long long v1) {
    PyObject* p1 = PyLong_FromLongLong(v1);
    if (!p1) return nullptr;
    PyObject* result = PyTuple_Pack(1, p1);
    Py_DECREF(p1);
    return result;
}

PyObject* jit_build_int_args2(long long v1, long long v2) {
    PyObject* p1 = PyLong_FromLongLong(v1);
    PyObject* p2 = PyLong_FromLongLong(v2);
    if (!p1 || !p2) { Py_XDECREF(p1); Py_XDECREF(p2); return nullptr; }
    PyObject* result = PyTuple_Pack(2, p1, p2);
    Py_DECREF(p1);
    Py_DECREF(p2);
    return result;
}

PyObject* jit_build_float_args1(double v1) {
    PyObject* p1 = PyFloat_FromDouble(v1);
    if (!p1) return nullptr;
    PyObject* result = PyTuple_Pack(1, p1);
    Py_DECREF(p1);
    return result;
}

PyObject* jit_build_float_args2(double v1, double v2) {
    PyObject* p1 = PyFloat_FromDouble(v1);
    PyObject* p2 = PyFloat_FromDouble(v2);
    if (!p1 || !p2) { Py_XDECREF(p1); Py_XDECREF(p2); return nullptr; }
    PyObject* result = PyTuple_Pack(2, p1, p2);
    Py_DECREF(p1);
    Py_DECREF(p2);
    return result;
}

// ============================================================================
// Iterator Support
// ============================================================================
PyObject* jit_iter_next(PyObject* iter) {
    return PyIter_Next(iter);
}

int jit_iter_check(PyObject* obj) {
    return PyIter_Check(obj);
}

PyObject* jit_get_iter(PyObject* obj) {
    return PyObject_GetIter(obj);
}

// ============================================================================
// Bytes/Bytearray Support
// ============================================================================
PyObject* jit_bytes_new(const char* data, Py_ssize_t len) {
    return PyBytes_FromStringAndSize(data, len);
}

const char* jit_bytes_data(PyObject* bytes) {
    if (!PyBytes_Check(bytes)) return nullptr;
    return PyBytes_AS_STRING(bytes);
}

Py_ssize_t jit_bytes_len(PyObject* bytes) {
    if (!PyBytes_Check(bytes)) return -1;
    return PyBytes_GET_SIZE(bytes);
}

// ============================================================================
// Python Expression Evaluation - Simplified API
// ============================================================================

// Evaluate a Python expression and return the result
PyObject* jit_py_eval(const char* expr, PyObject* locals) {
    PyObject* main_module = PyImport_AddModule("__main__");
    if (!main_module) return nullptr;
    
    PyObject* globals = PyModule_GetDict(main_module);
    if (!locals) locals = globals;
    
    return PyRun_String(expr, Py_eval_input, globals, locals);
}

// Execute Python statements (returns Py_None on success, NULL on error)
PyObject* jit_py_exec(const char* code, PyObject* locals) {
    PyObject* main_module = PyImport_AddModule("__main__");
    if (!main_module) return nullptr;
    
    PyObject* globals = PyModule_GetDict(main_module);
    if (!locals) locals = globals;
    
    PyObject* result = PyRun_String(code, Py_file_input, globals, locals);
    if (result) {
        Py_DECREF(result);
        Py_INCREF(Py_None);
        return Py_None;
    }
    return nullptr;
}

} // extern "C"

