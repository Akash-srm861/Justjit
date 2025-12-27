#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include "jit_core.h"

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(_core, m)
{
     // Disable leak warnings - our extension stores references to globals
     // which creates cycles that Python's GC handles but appear as leaks
     // at interpreter shutdown. See nanobind docs on reference leaks.
     nb::set_leak_warnings(false);

     m.doc() = "Fast Python JIT compiler using LLVM ORC";

     nb::class_<justjit::JITCore>(m, "JIT")
         .def(nb::init<>())
         .def("set_opt_level", &justjit::JITCore::set_opt_level, "level"_a)
         .def("get_opt_level", &justjit::JITCore::get_opt_level)
         .def("set_dump_ir", &justjit::JITCore::set_dump_ir, "dump"_a, "Enable/disable IR capture for debugging")
         .def("get_dump_ir", &justjit::JITCore::get_dump_ir, "Check if IR dump is enabled")
         .def("get_last_ir", &justjit::JITCore::get_last_ir, "Get the LLVM IR from the last compiled function")
         .def("compile", [](justjit::JITCore &self, nb::list instructions, nb::list constants, nb::list names, nb::object globals_dict, nb::object builtins_dict, nb::list closure_cells, nb::list exception_table, const std::string &name, int param_count, int total_locals, int nlocals)
              { return self.compile_function(instructions, constants, names, globals_dict, builtins_dict, closure_cells, exception_table, name, param_count, total_locals, nlocals); }, "instructions"_a, "constants"_a, "names"_a, "globals_dict"_a, "builtins_dict"_a, "closure_cells"_a, "exception_table"_a, "name"_a, "param_count"_a = 2, "total_locals"_a = 3, "nlocals"_a = 3, "Compile a Python function to native code")
         .def("compile_int", [](justjit::JITCore &self, nb::list instructions, nb::list constants, const std::string &name, int param_count, int total_locals)
              { return self.compile_int_function(instructions, constants, name, param_count, total_locals); }, "instructions"_a, "constants"_a, "name"_a, "param_count"_a = 2, "total_locals"_a = 3, "Compile an integer-only function to native code (no Python object overhead)")
         .def("compile_float", [](justjit::JITCore &self, nb::list instructions, nb::list constants, const std::string &name, int param_count, int total_locals)
              { return self.compile_float_function(instructions, constants, name, param_count, total_locals); }, "instructions"_a, "constants"_a, "name"_a, "param_count"_a = 2, "total_locals"_a = 3, "Compile a float-only function to native code (no Python object overhead)")
         .def("compile_generator", [](justjit::JITCore &self, nb::list instructions, nb::list constants, nb::list names, nb::object globals_dict, nb::object builtins_dict, nb::list closure_cells, nb::list exception_table, const std::string &name, int param_count, int total_locals, int nlocals)
              { return self.compile_generator(instructions, constants, names, globals_dict, builtins_dict, closure_cells, exception_table, name, param_count, total_locals, nlocals); }, "instructions"_a, "constants"_a, "names"_a, "globals_dict"_a, "builtins_dict"_a, "closure_cells"_a, "exception_table"_a, "name"_a, "param_count"_a = 0, "total_locals"_a = 1, "nlocals"_a = 1, "Compile a generator function to a state machine step function")
         .def("lookup", &justjit::JITCore::lookup_symbol, "name"_a)
         .def("get_callable", &justjit::JITCore::get_callable, "name"_a, "param_count"_a)
         .def("get_int_callable", &justjit::JITCore::get_int_callable, "name"_a, "param_count"_a, "Get a callable for an integer-mode function")
         .def("get_float_callable", &justjit::JITCore::get_float_callable, "name"_a, "param_count"_a, "Get a callable for a float-mode function")
         .def("compile_bool", [](justjit::JITCore &self, nb::list instructions, nb::list constants, const std::string &name, int param_count, int total_locals)
              { return self.compile_bool_function(instructions, constants, name, param_count, total_locals); }, "instructions"_a, "constants"_a, "name"_a, "param_count"_a = 2, "total_locals"_a = 3, "Compile a bool-only function to native code (no Python object overhead)")
         .def("get_bool_callable", &justjit::JITCore::get_bool_callable, "name"_a, "param_count"_a, "Get a callable for a bool-mode function")
         .def("compile_int32", [](justjit::JITCore &self, nb::list instructions, nb::list constants, const std::string &name, int param_count, int total_locals)
              { return self.compile_int32_function(instructions, constants, name, param_count, total_locals); }, "instructions"_a, "constants"_a, "name"_a, "param_count"_a = 2, "total_locals"_a = 3, "Compile a 32-bit integer function (C interop)")
         .def("get_int32_callable", &justjit::JITCore::get_int32_callable, "name"_a, "param_count"_a, "Get a callable for an int32-mode function")
         .def("compile_float32", [](justjit::JITCore &self, nb::list instructions, nb::list constants, const std::string &name, int param_count, int total_locals)
              { return self.compile_float32_function(instructions, constants, name, param_count, total_locals); }, "instructions"_a, "constants"_a, "name"_a, "param_count"_a = 2, "total_locals"_a = 3, "Compile a 32-bit float function (SIMD/ML)")
         .def("get_float32_callable", &justjit::JITCore::get_float32_callable, "name"_a, "param_count"_a, "Get a callable for a float32-mode function")
         .def("compile_complex128", [](justjit::JITCore &self, nb::list instructions, nb::list constants, const std::string &name, int param_count, int total_locals)
              { return self.compile_complex128_function(instructions, constants, name, param_count, total_locals); }, "instructions"_a, "constants"_a, "name"_a, "param_count"_a = 2, "total_locals"_a = 3, "Compile a complex128 function (scientific computing)")
         .def("get_complex128_callable", &justjit::JITCore::get_complex128_callable, "name"_a, "param_count"_a, "Get a callable for a complex128-mode function")
         .def("compile_ptr", [](justjit::JITCore &self, nb::list instructions, nb::list constants, const std::string &name, int param_count, int total_locals)
              { return self.compile_ptr_function(instructions, constants, name, param_count, total_locals); }, "instructions"_a, "constants"_a, "name"_a, "param_count"_a = 2, "total_locals"_a = 3, "Compile a ptr function (array access)")
         .def("get_ptr_callable", &justjit::JITCore::get_ptr_callable, "name"_a, "param_count"_a, "Get a callable for a ptr-mode function")
         .def("compile_vec4f", [](justjit::JITCore &self, nb::list instructions, nb::list constants, const std::string &name, int param_count, int total_locals)
              { return self.compile_vec4f_function(instructions, constants, name, param_count, total_locals); }, "instructions"_a, "constants"_a, "name"_a, "param_count"_a = 2, "total_locals"_a = 3, "Compile a vec4f function (SSE SIMD)")
         .def("get_vec4f_callable", &justjit::JITCore::get_vec4f_callable, "name"_a, "param_count"_a, "Get a callable for a vec4f-mode function")
         .def("compile_vec8i", [](justjit::JITCore &self, nb::list instructions, nb::list constants, const std::string &name, int param_count, int total_locals)
              { return self.compile_vec8i_function(instructions, constants, name, param_count, total_locals); }, "instructions"_a, "constants"_a, "name"_a, "param_count"_a = 2, "total_locals"_a = 3, "Compile a vec8i function (AVX SIMD)")
         .def("get_vec8i_callable", &justjit::JITCore::get_vec8i_callable, "name"_a, "param_count"_a, "Get a callable for a vec8i-mode function")
         .def("compile_complex64", [](justjit::JITCore &self, nb::list instructions, nb::list constants, const std::string &name, int param_count, int total_locals)
              { return self.compile_complex64_function(instructions, constants, name, param_count, total_locals); }, "instructions"_a, "constants"_a, "name"_a, "param_count"_a = 2, "total_locals"_a = 3, "Compile a complex64 function")
         .def("get_complex64_callable", &justjit::JITCore::get_complex64_callable, "name"_a, "param_count"_a, "Get a callable for a complex64-mode function")
         .def("compile_optional_f64", [](justjit::JITCore &self, nb::list instructions, nb::list constants, const std::string &name, int param_count, int total_locals)
              { return self.compile_optional_f64_function(instructions, constants, name, param_count, total_locals); }, "instructions"_a, "constants"_a, "name"_a, "param_count"_a = 2, "total_locals"_a = 3, "Compile an optional_f64 function")
         .def("get_optional_f64_callable", &justjit::JITCore::get_optional_f64_callable, "name"_a, "param_count"_a, "Get a callable for an optional_f64-mode function")
         .def("get_generator_callable", &justjit::JITCore::get_generator_callable, "name"_a, "param_count"_a, "total_locals"_a, "func_name"_a, "func_qualname"_a, "Get generator metadata for creating generator objects");

     // Expose the JITGenerator type and creation function
     m.def("create_jit_generator", [](uint64_t step_func_addr, int64_t num_locals, nb::object name, nb::object qualname) {
         auto step_func = reinterpret_cast<justjit::GeneratorStepFunc>(step_func_addr);
         PyObject* gen = justjit::JITGenerator_New(step_func, static_cast<Py_ssize_t>(num_locals),
                                                    name.ptr(), qualname.ptr());
         if (gen == nullptr) {
             throw nb::python_error();
         }
         return nb::steal(gen);
     }, "step_func_addr"_a, "num_locals"_a, "name"_a, "qualname"_a,
        "Create a new JIT generator object from a compiled step function");
     
     // Expose the JITCoroutine type and creation function
     m.def("create_jit_coroutine", [](uint64_t step_func_addr, int64_t num_locals, nb::object name, nb::object qualname) {
         auto step_func = reinterpret_cast<justjit::GeneratorStepFunc>(step_func_addr);
         PyObject* coro = justjit::JITCoroutine_New(step_func, static_cast<Py_ssize_t>(num_locals),
                                                     name.ptr(), qualname.ptr());
         if (coro == nullptr) {
             throw nb::python_error();
         }
         return nb::steal(coro);
     }, "step_func_addr"_a, "num_locals"_a, "name"_a, "qualname"_a,
        "Create a new JIT coroutine object from a compiled step function");
}
