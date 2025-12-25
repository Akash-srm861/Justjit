#pragma once

// Generator-specific opcode handlers - separated to avoid MSVC nesting limit
// These are included inline in compile_generator()

#include "opcodes.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/BasicBlock.h>

namespace justjit {

// Handle ENTER_EXECUTOR opcode in generator
// Returns true if handled
inline bool handle_enter_executor_gen(
    const uint8_t opcode,
    llvm::IRBuilder<>& builder)
{
    if (opcode != op::ENTER_EXECUTOR) return false;
    // ENTER_EXECUTOR: CPython Tier 2 JIT entry point signal
    // For our JIT, this is a no-op - we are already the executor
    return true;
}

// Handle INTERPRETER_EXIT opcode in generator
// Returns true if handled
inline bool handle_interpreter_exit_gen(
    const uint8_t opcode,
    llvm::IRBuilder<>& builder,
    llvm::Value* state_ptr,
    llvm::Type* i32_type,
    llvm::LLVMContext& context,
    llvm::Function* func,
    int offset)
{
    if (opcode != op::INTERPRETER_EXIT) return false;
    
    // INTERPRETER_EXIT: Force exit from the evaluation loop
    // For generators, return NULL to signal exit and set state to -2 (error)
    builder.CreateStore(llvm::ConstantInt::get(i32_type, -2), state_ptr);
    builder.CreateRet(llvm::ConstantPointerNull::get(llvm::PointerType::get(context, 0)));
    
    // Create unreachable block for following code
    llvm::BasicBlock *unreachable_bb = llvm::BasicBlock::Create(
        context, "after_exit_" + std::to_string(offset), func);
    builder.SetInsertPoint(unreachable_bb);
    
    return true;
}

// Handle STORE_FAST_MAYBE_NULL opcode in generator
// Returns true if handled
inline bool handle_store_fast_maybe_null_gen(
    const uint8_t opcode,
    uint16_t arg,
    std::vector<llvm::Value*>& stack,
    llvm::IRBuilder<>& builder,
    llvm::Value* locals_array,
    llvm::Type* ptr_type,
    llvm::Type* i64_type,
    llvm::LLVMContext& context,
    llvm::Function* func,
    llvm::FunctionCallee py_xdecref_func,
    int total_locals,
    size_t instr_idx)
{
    if (opcode != op::STORE_FAST_MAYBE_NULL) return false;
    
    // STORE_FAST_MAYBE_NULL: Same as STORE_FAST but slot may be uninitialized
    // Our STORE_FAST implementation already handles NULL safely
    if (!stack.empty())
    {
        llvm::Value *val = stack.back();
        stack.pop_back();

        int local_idx = arg;
        if (local_idx < total_locals)
        {
            llvm::Value *slot_idx = llvm::ConstantInt::get(i64_type, local_idx);
            llvm::Value *slot_ptr = builder.CreateGEP(ptr_type, locals_array, slot_idx);
            
            // Decref old value before storing (NULL-safe)
            llvm::Value *old_val = builder.CreateLoad(ptr_type, slot_ptr, "old_local");
            llvm::Value *null_check = llvm::ConstantPointerNull::get(llvm::PointerType::get(context, 0));
            llvm::Value *is_not_null = builder.CreateICmpNE(old_val, null_check, "is_not_null");
            
            llvm::BasicBlock *decref_bb = llvm::BasicBlock::Create(context, "decref_old_" + std::to_string(instr_idx), func);
            llvm::BasicBlock *store_bb = llvm::BasicBlock::Create(context, "store_new_" + std::to_string(instr_idx), func);
            
            builder.CreateCondBr(is_not_null, decref_bb, store_bb);
            
            builder.SetInsertPoint(decref_bb);
            builder.CreateCall(py_xdecref_func, {old_val});
            builder.CreateBr(store_bb);
            
            builder.SetInsertPoint(store_bb);
            builder.CreateStore(val, slot_ptr);
        }
    }
    
    return true;
}

} // namespace justjit
