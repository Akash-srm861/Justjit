#pragma once

// Generator opcode handlers - Batch 2
// Contains CALL_INTRINSIC_2, SETUP_ANNOTATIONS and other handlers
// Separated to avoid MSVC C1061 nesting limit

#include "opcodes.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/BasicBlock.h>

namespace justjit {

// Handle CALL_INTRINSIC_2 opcode in generator
// Returns true if handled
inline bool handle_call_intrinsic_2_gen(
    const uint8_t opcode,
    uint16_t arg,
    std::vector<llvm::Value*>& stack,
    llvm::IRBuilder<>& builder,
    llvm::Type* ptr_type,
    llvm::Type* i64_type,
    llvm::FunctionCallee py_object_setattr_func,
    llvm::FunctionCallee py_xdecref_func)
{
    if (opcode != op::CALL_INTRINSIC_2) return false;
    
    if (stack.size() >= 2)
    {
        llvm::Value *arg2 = stack.back(); stack.pop_back();
        llvm::Value *arg1 = stack.back(); stack.pop_back();
        
        int intrinsic = arg;
        llvm::Value *result = nullptr;
        
        if (intrinsic == 4)
        {
            // SET_FUNCTION_TYPE_PARAMS
            PyObject *type_params_str = PyUnicode_InternFromString("__type_params__");
            llvm::Value *attr_name = builder.CreateIntToPtr(
                llvm::ConstantInt::get(i64_type, reinterpret_cast<uint64_t>(type_params_str)), ptr_type);
            builder.CreateCall(py_object_setattr_func, {arg1, attr_name, arg2});
            builder.CreateCall(py_xdecref_func, {arg2});
            result = arg1;
        }
        else
        {
            // Default: treat as no-op, return arg1
            builder.CreateCall(py_xdecref_func, {arg2});
            result = arg1;
        }
        
        stack.push_back(result);
    }
    
    return true;
}

// Handle SETUP_ANNOTATIONS opcode in generator
// Returns true if handled
inline bool handle_setup_annotations_gen(
    const uint8_t opcode,
    llvm::IRBuilder<>& builder,
    llvm::Type* ptr_type,
    llvm::Type* i64_type,
    llvm::LLVMContext& context,
    llvm::Function* func,
    PyObject* globals_dict_ptr,
    llvm::FunctionCallee py_dict_getitem_func,
    llvm::FunctionCallee py_dict_new_func,
    llvm::FunctionCallee py_dict_setitem_func,
    llvm::FunctionCallee py_xdecref_func)
{
    if (opcode != op::SETUP_ANNOTATIONS) return false;
    
    // Create __annotations__ dict if not exists in globals
    PyObject *annot_str = PyUnicode_InternFromString("__annotations__");
    Py_INCREF(annot_str);  // Keep alive
    
    llvm::Value *annot_name = builder.CreateIntToPtr(
        llvm::ConstantInt::get(i64_type, reinterpret_cast<uint64_t>(annot_str)), ptr_type);
    llvm::Value *globals_dict = builder.CreateIntToPtr(
        llvm::ConstantInt::get(i64_type, reinterpret_cast<uint64_t>(globals_dict_ptr)), ptr_type);
    
    llvm::Value *existing = builder.CreateCall(py_dict_getitem_func, {globals_dict, annot_name});
    llvm::Value *is_null = builder.CreateICmpEQ(existing, 
        llvm::ConstantPointerNull::get(llvm::PointerType::get(context, 0)));
    
    llvm::BasicBlock *create_block = llvm::BasicBlock::Create(context, "create_annot", func);
    llvm::BasicBlock *done_block = llvm::BasicBlock::Create(context, "annot_done", func);
    
    builder.CreateCondBr(is_null, create_block, done_block);
    
    builder.SetInsertPoint(create_block);
    llvm::Value *new_dict = builder.CreateCall(py_dict_new_func, {});
    builder.CreateCall(py_dict_setitem_func, {globals_dict, annot_name, new_dict});
    builder.CreateCall(py_xdecref_func, {new_dict});
    builder.CreateBr(done_block);
    
    builder.SetInsertPoint(done_block);
    
    return true;
}

// Handle BEFORE_ASYNC_WITH opcode in generator
// Returns true if handled (but actual logic stays in main file for complexity)
inline bool is_before_async_with(const uint8_t opcode) {
    return opcode == op::BEFORE_ASYNC_WITH;
}

// Handle LOAD_BUILD_CLASS opcode in generator
// Returns true if handled
inline bool handle_load_build_class_gen(
    const uint8_t opcode,
    std::vector<llvm::Value*>& stack,
    llvm::IRBuilder<>& builder,
    llvm::Type* ptr_type,
    llvm::Type* i64_type,
    PyObject* builtins_dict_ptr,
    llvm::FunctionCallee py_dict_getitem_func,
    llvm::FunctionCallee py_incref_func)
{
    if (opcode != op::LOAD_BUILD_CLASS) return false;
    
    PyObject *build_class_str = PyUnicode_InternFromString("__build_class__");
    Py_INCREF(build_class_str);
    
    llvm::Value *name_ptr = builder.CreateIntToPtr(
        llvm::ConstantInt::get(i64_type, reinterpret_cast<uint64_t>(build_class_str)), ptr_type);
    llvm::Value *builtins_dict = builder.CreateIntToPtr(
        llvm::ConstantInt::get(i64_type, reinterpret_cast<uint64_t>(builtins_dict_ptr)), ptr_type);
    
    llvm::Value *build_class = builder.CreateCall(py_dict_getitem_func, {builtins_dict, name_ptr});
    builder.CreateCall(py_incref_func, {build_class});
    
    stack.push_back(build_class);
    
    return true;
}

// Handle EXIT_INIT_CHECK opcode in generator
// Returns true if handled
inline bool handle_exit_init_check_gen(
    const uint8_t opcode,
    std::vector<llvm::Value*>& stack,
    llvm::IRBuilder<>& builder,
    llvm::Type* ptr_type,
    llvm::LLVMContext& context,
    llvm::Function* func,
    llvm::FunctionCallee py_xdecref_func)
{
    if (opcode != op::EXIT_INIT_CHECK) return false;
    
    if (!stack.empty())
    {
        llvm::Value *should_be_none = stack.back();
        stack.pop_back();
        
        // Check if returned value from __init__ is not None
        llvm::Value *py_none = builder.CreateIntToPtr(
            llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 
                reinterpret_cast<uint64_t>(Py_None)), ptr_type);
        
        llvm::Value *is_none = builder.CreateICmpEQ(should_be_none, py_none);
        
        llvm::BasicBlock *ok_block = llvm::BasicBlock::Create(context, "init_ok", func);
        llvm::BasicBlock *err_block = llvm::BasicBlock::Create(context, "init_err", func);
        
        builder.CreateCondBr(is_none, ok_block, err_block);
        
        // Error path - just continue for now
        builder.SetInsertPoint(err_block);
        builder.CreateCall(py_xdecref_func, {should_be_none});
        builder.CreateBr(ok_block);
        
        builder.SetInsertPoint(ok_block);
    }
    
    return true;
}

} // namespace justjit
