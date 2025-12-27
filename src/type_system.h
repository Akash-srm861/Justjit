#pragma once

// JustJIT Type System
// Defines native types for Box/Unbox operations and typed compilation

#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>
#include <cstdint>

namespace justjit
{

// Native type identifiers for Box/Unbox
enum class JITType : uint8_t
{
    OBJECT = 0,   // PyObject* (default, full Python semantics)
    INT64 = 1,    // int64_t (native signed integer)
    FLOAT64 = 2,  // double (native floating point)
    BOOL = 3,     // bool (i1, true/false)
    UINT64 = 4,   // uint64_t (native unsigned integer)
    INT32 = 5,    // int32_t (32-bit signed, C interop)
    FLOAT32 = 6,  // float (32-bit float, SIMD/ML)
    COMPLEX128 = 7, // {double, double} (complex number, real + imag)
    PTR_F64 = 8,  // ptr to double array (array operations)
    VEC4F = 9,    // <4 x float> (SSE SIMD)
    VEC8I = 10,   // <8 x i32> (AVX SIMD)
    COMPLEX64 = 11, // {float, float} (single-precision complex)
    OPTIONAL_F64 = 12, // {i1, f64} (nullable float64)
};

// Convert JITType to LLVM Type
inline llvm::Type* jit_type_to_llvm(JITType type, llvm::LLVMContext& ctx)
{
    switch (type)
    {
    case JITType::INT64:
    case JITType::UINT64:
        return llvm::Type::getInt64Ty(ctx);
    case JITType::FLOAT64:
        return llvm::Type::getDoubleTy(ctx);
    case JITType::BOOL:
        return llvm::Type::getInt1Ty(ctx);
    case JITType::OBJECT:
    default:
        return llvm::PointerType::getUnqual(ctx); // PyObject*
    }
}

// Check if type is a native (non-object) type
inline bool is_native_type(JITType type)
{
    return type != JITType::OBJECT;
}

// Structure to hold a typed value during compilation
struct TypedValue
{
    llvm::Value* value;
    JITType type;
    
    TypedValue() : value(nullptr), type(JITType::OBJECT) {}
    TypedValue(llvm::Value* v, JITType t) : value(v), type(t) {}
    
    bool is_native() const { return is_native_type(type); }
    bool is_int() const { return type == JITType::INT64 || type == JITType::UINT64; }
    bool is_float() const { return type == JITType::FLOAT64; }
    bool is_bool() const { return type == JITType::BOOL; }
    bool is_object() const { return type == JITType::OBJECT; }
};

// Range loop info for native FOR_ITER optimization
struct RangeLoopInfo
{
    int64_t start;      // range start value (default 0)
    int64_t stop;       // range stop value
    int64_t step;       // range step value (default 1)
    int loop_var_idx;   // local variable index for loop counter
    int body_start;     // bytecode offset of loop body
    int loop_end;       // bytecode offset after loop (END_FOR target)
    bool is_valid;      // true if pattern was successfully detected
    
    RangeLoopInfo() : start(0), stop(0), step(1), loop_var_idx(-1), 
                      body_start(-1), loop_end(-1), is_valid(false) {}
};

} // namespace justjit
