# CFG Analysis and PHI Node Implementation Plan

## Executive Summary

The current JIT compiler uses a single compile-time `std::vector<llvm::Value*> stack` to simulate Python's value stack. This approach breaks when bytecode contains conditional branches because:

1. Different branches may push/pop different values
2. At merge points, LLVM doesn't know which values exist
3. This causes "Instruction does not dominate all uses" errors

The solution requires implementing proper Control Flow Graph (CFG) analysis with per-block stack tracking and PHI nodes at merge points.

---

## Phase 1: CFG Construction and Analysis

### 1.1 Define Basic Block Structure

Create a new data structure to represent basic blocks:

```cpp
struct BasicBlockInfo {
    int start_offset;           // First bytecode offset in this block
    int end_offset;             // Last bytecode offset (exclusive)
    llvm::BasicBlock* llvm_bb;  // Corresponding LLVM basic block
    
    // Control flow edges
    std::vector<int> predecessors;  // Offsets of predecessor blocks
    std::vector<int> successors;    // Offsets of successor blocks
    
    // Stack state
    int entry_stack_depth;          // Expected stack depth at entry (-1 = unknown)
    int exit_stack_depth;           // Stack depth at exit
    std::vector<llvm::Value*> entry_stack;  // Values on stack at entry
    std::vector<llvm::Value*> exit_stack;   // Values on stack at exit
    
    // PHI nodes for stack slots that need reconciliation
    std::vector<llvm::PHINode*> stack_phis;
    
    // Flags
    bool is_entry;              // Is this the function entry point?
    bool is_loop_header;        // Is this a loop header (has back edge)?
    bool is_exception_handler;  // Is this an exception handler entry?
    bool processed;             // Has this block been compiled?
};
```

### 1.2 Identify Block Boundaries

Scan bytecode to identify where blocks begin/end:

**Block starts at offset N when:**
1. N == 0 (function entry)
2. N is target of any jump (forward, backward, conditional)
3. N is exception handler target
4. N follows a branch/jump instruction (fall-through)

**Block ends when:**
1. A branch instruction (POP_JUMP_IF_*, JUMP_FORWARD, JUMP_BACKWARD)
2. A return instruction (RETURN_VALUE, RETURN_CONST)
3. An instruction that terminates the block (RAISE_VARARGS)
4. The next instruction starts a new block

### 1.3 Build CFG Edges

For each block, determine successors:

| Instruction Type | Successors |
|-----------------|------------|
| RETURN_VALUE/RETURN_CONST | None (terminal) |
| JUMP_FORWARD | Target only |
| JUMP_BACKWARD | Target only |
| POP_JUMP_IF_FALSE/TRUE | Target AND fall-through |
| POP_JUMP_IF_NONE/NOT_NONE | Target AND fall-through |
| FOR_ITER | fall-through AND target (loop exit) |
| Normal instruction | Fall-through |

### 1.4 Compute Dominator Tree (Optional but useful)

For optimization and correctness verification:
- Use LLVM's built-in dominator tree computation
- Helps verify that values are only used where they dominate

---

## Phase 2: Stack State Tracking

### 2.1 Abstract Stack State

Instead of tracking actual `llvm::Value*` at compile time, track abstract stack slots:

```cpp
struct StackSlot {
    int slot_id;                // Unique ID for this stack position
    llvm::PHINode* phi;         // PHI node (if at merge point)
    llvm::Value* value;         // Actual value (if single-source)
    llvm::Type* type;           // Expected type (ptr for objects, i64 for ints)
};
```

### 2.2 Stack Depth Verification

Python bytecode is verified, so stack depths at merge points are guaranteed equal. We need to:

1. **Entry block**: Stack depth = 0
2. **Branch targets**: Verify all predecessors have same exit depth
3. **Loop headers**: Verify back-edge and entry-edge have same depth

### 2.3 Value Reconciliation at Merge Points

When a block has multiple predecessors with potentially different values at the same stack position:

```cpp
// Pseudocode for creating stack PHIs
void create_stack_phis(BasicBlockInfo& block) {
    if (block.predecessors.size() <= 1) {
        // No merge needed, just copy from predecessor
        return;
    }
    
    int stack_depth = block.entry_stack_depth;
    block.stack_phis.resize(stack_depth);
    
    for (int slot = 0; slot < stack_depth; slot++) {
        llvm::PHINode* phi = builder.CreatePHI(ptr_type, 
                                                block.predecessors.size(),
                                                "stack_" + std::to_string(slot));
        block.stack_phis[slot] = phi;
        block.entry_stack[slot] = phi;  // Use PHI as the value
    }
}

// After all predecessors are processed, fill in PHI incoming values
void finalize_stack_phis(BasicBlockInfo& block, 
                         std::map<int, BasicBlockInfo>& blocks) {
    for (int slot = 0; slot < block.stack_phis.size(); slot++) {
        for (int pred_offset : block.predecessors) {
            BasicBlockInfo& pred = blocks[pred_offset];
            llvm::Value* pred_value = pred.exit_stack[slot];
            block.stack_phis[slot]->addIncoming(pred_value, pred.llvm_bb);
        }
    }
}
```

---

## Phase 3: Two-Pass Compilation

### 3.1 First Pass: Block Discovery and PHI Creation

```cpp
// Pass 1: Discover blocks, create LLVM basic blocks, create PHI nodes
void first_pass(const std::vector<Instruction>& instructions) {
    // 1. Find all block boundaries
    std::set<int> block_starts = find_block_starts(instructions);
    
    // 2. Create BasicBlockInfo for each block
    for (int start : block_starts) {
        BasicBlockInfo info;
        info.start_offset = start;
        info.llvm_bb = llvm::BasicBlock::Create(context, "block_" + std::to_string(start), func);
        blocks[start] = info;
    }
    
    // 3. Compute CFG edges (predecessors/successors)
    compute_cfg_edges(instructions);
    
    // 4. Compute stack depths at each block entry
    compute_stack_depths();
    
    // 5. Create PHI nodes for merge points
    for (auto& [offset, block] : blocks) {
        if (block.predecessors.size() > 1) {
            create_stack_phis(block);
        }
    }
}
```

### 3.2 Second Pass: Code Generation

Process blocks in topological order (predecessors before successors, except back-edges):

```cpp
void second_pass(const std::vector<Instruction>& instructions) {
    std::vector<int> worklist = topological_sort(blocks);
    
    for (int block_offset : worklist) {
        BasicBlockInfo& block = blocks[block_offset];
        builder.SetInsertPoint(block.llvm_bb);
        
        // Initialize stack from entry_stack (may be PHIs)
        std::vector<llvm::Value*> stack = block.entry_stack;
        
        // Process instructions in this block
        for (int offset = block.start_offset; offset < block.end_offset; ) {
            const Instruction& instr = get_instruction_at(offset);
            compile_instruction(instr, stack);
            offset = next_offset(offset);
        }
        
        // Save exit stack for successor blocks
        block.exit_stack = stack;
    }
    
    // Finalize PHI nodes
    for (auto& [offset, block] : blocks) {
        if (block.stack_phis.size() > 0) {
            finalize_stack_phis(block, blocks);
        }
    }
}
```

### 3.3 Topological Sort with Back-Edge Handling

For loops, we need special handling:

```cpp
std::vector<int> topological_sort(std::map<int, BasicBlockInfo>& blocks) {
    std::vector<int> result;
    std::set<int> visited;
    std::set<int> in_stack;  // For cycle detection
    
    std::function<void(int)> visit = [&](int offset) {
        if (visited.count(offset)) return;
        if (in_stack.count(offset)) {
            // Back edge detected - this is a loop
            blocks[offset].is_loop_header = true;
            return;
        }
        
        in_stack.insert(offset);
        for (int succ : blocks[offset].successors) {
            visit(succ);
        }
        in_stack.erase(offset);
        
        visited.insert(offset);
        result.push_back(offset);
    };
    
    visit(0);  // Start from entry block
    
    std::reverse(result.begin(), result.end());
    return result;
}
```

---

## Phase 4: Local Variable Handling

### 4.1 Current Approach (Keep)

Local variables already use LLVM allocas, which work correctly across branches. No changes needed for locals.

### 4.2 Stack vs Locals Distinction

- **Stack slots**: Need PHIs at merge points (ephemeral values)
- **Local variables**: Use allocas (persistent storage)

---

## Phase 5: Special Cases

### 5.1 FOR_ITER Pattern

```python
for x in iterable:
    body
```

Compiles to:
```
GET_ITER
FOR_ITER target  # Pushes next value or jumps to target
STORE_FAST x
... body ...
JUMP_BACKWARD
target:
END_FOR  # Pops exhausted iterator
```

The FOR_ITER instruction:
- Success path: pushes one value (next item)
- Failure path: jumps to target with iterator still on stack

This is handled correctly if we track:
- Entry to loop body has stack depth +1 (iterator + item)
- Entry to END_FOR has stack depth (iterator only)

### 5.2 Exception Handlers

Exception handlers have a specific stack state:
- Stack depth from exception table entry
- TOS is the exception value

This is already partially handled via `exception_handler_depth` map.

### 5.3 Pattern Matching

With proper CFG/PHI support, pattern matching becomes trivial:
- MATCH_MAPPING/MATCH_SEQUENCE push True/False
- POP_JUMP_IF_FALSE branches
- Different paths have different stack states
- PHI nodes reconcile at merge points

---

## Phase 6: Implementation Order

### Step 1: Add CFG Data Structures (Low Risk)
- Add `BasicBlockInfo` struct to `jit_core.h`
- Add `std::map<int, BasicBlockInfo> blocks` member to `JITCore`
- No changes to existing compilation yet

### Step 2: Implement Block Discovery (Low Risk)
- Create `find_block_starts()` function
- Create `compute_cfg_edges()` function
- Test that blocks are correctly identified

### Step 3: Implement Stack Depth Analysis (Medium Risk)
- Create `compute_stack_depths()` function
- Verify against Python's guaranteed stack depths
- Add assertions to catch mismatches

### Step 4: Create Parallel Compilation Path (Medium Risk)
- Add `compile_function_v2()` that uses CFG-based approach
- Keep existing `compile_function()` for fallback
- Test both paths produce identical results for simple cases

### Step 5: Implement PHI Node Generation (High Complexity)
- Add `create_stack_phis()` and `finalize_stack_phis()`
- Test with simple branching code (if/else)
- Test with loops

### Step 6: Enable for Pattern Matching (Validation)
- Remove pattern matching from blocked list
- Run pattern matching tests
- Verify all patterns work correctly

### Step 7: Migration and Cleanup
- Replace old compilation path with new CFG-based path
- Remove legacy `block_incoming_stacks` tracking
- Update documentation

---

## Phase 7: Testing Strategy

### Unit Tests
1. **Block discovery**: Verify correct block boundaries for various bytecode patterns
2. **CFG edges**: Verify predecessor/successor relationships
3. **Stack depths**: Verify depths match Python's guarantees
4. **PHI nodes**: Verify values are correctly reconciled

### Integration Tests
1. **Simple if/else**: Two-way branch and merge
2. **Nested if/else**: Multiple nesting levels
3. **For loops**: Loop with break/continue
4. **While loops**: Condition-first loops
5. **Pattern matching**: All match types (mapping, sequence, class, keys)
6. **Exception handling**: Try/except with branches

### Regression Tests
- Run existing 108-test suite
- Run pattern matching tests
- Run any additional test files

---

## Estimated Effort

| Phase | Description | Estimated Time | Risk Level |
|-------|-------------|----------------|------------|
| 1 | CFG Construction | 2-3 hours | Low |
| 2 | Stack State Tracking | 2-3 hours | Medium |
| 3 | Two-Pass Compilation | 4-6 hours | High |
| 4 | Local Variable Handling | 1 hour | Low |
| 5 | Special Cases | 2-3 hours | Medium |
| 6 | Implementation | 8-12 hours | High |
| 7 | Testing | 3-4 hours | Medium |

**Total: ~25-35 hours of focused work**

---

## Alternative Approaches Considered

### A. Use LLVM's MemorySSA
- Pro: Mature infrastructure
- Con: Designed for memory, not stack simulation

### B. Use LLVM's Mem2Reg Pass
- Pro: Converts allocas to SSA automatically
- Con: Would require allocating stack slots as allocas

### C. Store stack in allocas (like locals)
- Pro: Simpler, no PHI management needed
- Con: Performance overhead, more LLVM IR

### D. Interpreter fallback for complex CFG
- Pro: Simple to implement
- Con: Defeats purpose of JIT

**Recommendation**: The proposed two-pass PHI approach is the cleanest solution for a bytecode-based JIT. It matches how real VMs (like V8, HotSpot) handle this problem.

---

## Code Sketch: Key Functions

### find_block_starts()

```cpp
std::set<int> find_block_starts(const std::vector<Instruction>& instructions) {
    std::set<int> starts;
    starts.insert(0);  // Function entry
    
    for (size_t i = 0; i < instructions.size(); i++) {
        const auto& instr = instructions[i];
        
        // Jump targets start blocks
        if (is_jump_instruction(instr.opcode)) {
            starts.insert(instr.argval);
        }
        
        // Instruction after branch/jump starts a block
        if (is_branch_instruction(instr.opcode) && i + 1 < instructions.size()) {
            starts.insert(instructions[i + 1].offset);
        }
        
        // Return terminates (no fall-through block needed)
    }
    
    return starts;
}
```

### compute_stack_depths()

```cpp
void compute_stack_depths() {
    std::queue<int> worklist;
    worklist.push(0);
    blocks[0].entry_stack_depth = 0;
    
    while (!worklist.empty()) {
        int offset = worklist.front();
        worklist.pop();
        
        BasicBlockInfo& block = blocks[offset];
        int depth = block.entry_stack_depth;
        
        // Simulate stack effects of all instructions in block
        for (/* each instruction in block */) {
            depth += stack_effect(instr);
        }
        
        block.exit_stack_depth = depth;
        
        // Propagate to successors
        for (int succ : block.successors) {
            if (blocks[succ].entry_stack_depth == -1) {
                blocks[succ].entry_stack_depth = depth;
                worklist.push(succ);
            } else {
                assert(blocks[succ].entry_stack_depth == depth);
            }
        }
    }
}
```

---

## Conclusion

This plan provides a systematic approach to implementing proper CFG analysis with PHI nodes. The two-pass compilation strategy cleanly separates concerns:

1. **First pass**: Structure discovery (blocks, edges, depths, PHI creation)
2. **Second pass**: Code generation (instruction compilation, value tracking)

This architecture will not only fix pattern matching but also improve reliability for all branching code and enable future optimizations like loop invariant code motion.
