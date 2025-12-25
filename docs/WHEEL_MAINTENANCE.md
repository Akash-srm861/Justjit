# JustJIT Wheel Build & Maintenance Guide

This document explains how to maintain cross-platform wheel builds and avoid common pitfalls when working with Python C API and LLVM.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                     Python Layer                             │
│   @justjit.jit decorator → bytecode extraction → JIT call   │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                   Native Extension (_core)                   │
│   jit_core.cpp ─────────────────────────────────────────────│
│   ├── JITCore class (LLVM OrcJIT wrapper)                   │
│   ├── compile_function() - Regular function compilation     │
│   ├── compile_generator() - Generator state machine         │
│   └── Helper functions (JITGetAwaitable, etc.)              │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      LLVM Libraries                          │
│   libLLVM.so (Linux) / libLLVM.dylib (macOS) / LLVM.dll     │
└─────────────────────────────────────────────────────────────┘
```

## Critical Rules for Cross-Platform Compatibility

### 1. Python C API Usage

**Never use internal APIs (underscore-prefixed functions/types)**

| ❌ Avoid | ✅ Use Instead |
|----------|----------------|
| `PyCoro_CheckExact(obj)` | `strcmp(Py_TYPE(obj)->tp_name, "coroutine") == 0` |
| `PyGen_GetCode(gen)` | `PyObject_GetAttrString(obj, "gi_code")` |
| `PyObject_CallOneArg(f, a)` | `PyObject_CallFunctionObjArgs(f, a, NULL)` |
| `_Py*` functions | Public `Py*` equivalents |

**Reason:** Internal APIs may not be exported as symbols on all platforms, especially macOS.

### 2. Symbol Naming

**Never start C function names with underscore**

```cpp
// ❌ Bad - macOS prepends underscore, creates __PyJIT_* symbol issues
extern "C" PyObject *_PyJIT_GetAwaitable(PyObject *obj);

// ✅ Good
extern "C" PyObject *JITGetAwaitable(PyObject *obj);
```

### 3. LLVM Linking

**Conda-forge llvmdev provides DYNAMIC libraries only**

- `libLLVM.so` / `libLLVM.dylib` / `LLVM.dll`
- NOT static `.a` files

**CMakeLists.txt pattern:**
```cmake
find_package(LLVM REQUIRED CONFIG)
# Link against single shared library
find_library(LLVM_SHARED_LIB NAMES LLVM LLVM-${LLVM_VERSION_MAJOR} 
             PATHS ${LLVM_LIBRARY_DIR} NO_DEFAULT_PATH)
target_link_libraries(_core PRIVATE ${LLVM_SHARED_LIB})
```

### 4. Wheel Repair Tools

Each platform needs the LLVM shared library bundled:

| Platform | Tool | Command |
|----------|------|---------|
| Linux | auditwheel | `auditwheel repair --plat manylinux_2_28_x86_64 -w {dest_dir} {wheel}` |
| macOS | delocate | `delocate-wheel --require-archs {delocate_archs} -w {dest_dir} {wheel}` |
| Windows | delvewheel | `delvewheel repair --add-path "LLVM_BIN_DIR" -w {dest_dir} {wheel}` |

**Verification commands:**
```bash
# Check if LLVM is bundled
auditwheel show dist/*.whl          # Linux
delocate-listdeps dist/*.whl        # macOS
delvewheel show dist\*.whl          # Windows
```

## File Structure

```
src/
├── jit_core.cpp      # Main JIT implementation (11K+ lines)
├── jit_core.h        # JITCore class definition
├── opcodes.h         # Python opcode definitions
├── bindings.cpp      # nanobind Python bindings
├── gen_opcodes.h     # Generated opcode handlers
└── gen_opcodes_batch2.h
```

## Key Components in jit_core.cpp

### Helper Functions (lines ~100-300)
```cpp
extern "C" void jit_xincref(PyObject *obj);
extern "C" void jit_xdecref(PyObject *obj);
extern "C" PyObject *JITGetAwaitable(PyObject *obj);
extern "C" PyObject *JITMatchKeys(PyObject *subject, PyObject *keys);
extern "C" PyObject *JITMatchClass(PyObject *subject, PyObject *cls, int nargs, PyObject *names);
```

### JITCore Class (lines ~300-500)
- LLVM context, JIT engine initialization
- Symbol registration for helper functions
- Module optimization settings

### compile_function() (lines ~500-8000)
- Bytecode to LLVM IR translation
- Stack simulation
- Opcode handling (LOAD_*, STORE_*, BINARY_OP, etc.)

### compile_generator() (lines ~8000-10000)
- State machine construction
- YIELD_VALUE handling with stack spilling
- State dispatch via LLVM switch

### JIT Wrapper Types (lines ~11000+)
- JITFunctionObject
- JITGeneratorObject  
- JITCoroutineObject

## Adding New Opcode Support

1. Add opcode constant to `opcodes.h`
2. Add handler in `compile_function()` switch statement:
   ```cpp
   case op::NEW_OPCODE:
   {
       // Generate LLVM IR for the opcode
       break;
   }
   ```
3. If opcode needs runtime helper, add to helper functions section
4. Register helper in JITCore constructor's `helper_symbols` map
5. Update `docs/OPCODES_REFERENCE.md`

## CI/CD Workflow (wheels.yml)

### Build Matrix
- Linux: manylinux_2_28 (x86_64)
- macOS: macos-14 (arm64)
- Windows: windows-latest (AMD64)

### Python Version
- Currently: Python 3.13 only (`cp313-*`)

### LLVM Source
- macOS/Windows: conda-forge `llvmdev`
- Linux: System LLVM from EPEL/CRB

## Troubleshooting

### Undefined Symbol Errors
1. Check if using internal Python API → Replace with public API
2. Check function name starts with underscore → Rename
3. Check LLVM linked correctly → Verify `find_library()` succeeds

### Wheel Not Importing
```bash
# Verify dependencies bundled
python -c "import justjit; print('OK')"

# On failure, check ldd/otool/dumpbin output
ldd justjit/_core.*.so         # Linux
otool -L justjit/_core.*.so    # macOS
dumpbin /DEPENDENTS _core.pyd  # Windows
```

### auditwheel Fails
- Ensure `LD_LIBRARY_PATH` includes LLVM lib directory during repair
- Check GLIBC version compatibility with manylinux tag

## Version Compatibility Matrix

| justjit | Python | LLVM | Notes |
|---------|--------|------|-------|
| 0.1.3 | 3.13 | 18+ | Current release |

## Future Improvements

1. **Static LLVM linking** - Requires building LLVM from source with `-DBUILD_SHARED_LIBS=OFF`
2. **Multi-Python support** - Add back cp310, cp311, cp312 builds
3. **macOS Intel support** - Re-add macos-13 (x86_64) to matrix
4. **Native FOR_ITER** - Implement range() loop unrolling in integer mode
