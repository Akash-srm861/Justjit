# Python 3.13 Bytecode Opcodes Reference

Complete reference for Python 3.13 bytecode opcodes in JustJIT.

**Source:** [Python 3.13 dis module documentation](https://docs.python.org/3.13/library/dis.html)

---

## Table of Contents

1. [Implemented Opcodes (65+)](#implemented-opcodes)
2. [Missing Opcodes](#missing-opcodes)
3. [Implementation Details](#implementation-details)

---

## Implemented Opcodes

JustJIT implements 65+ opcodes across three compilation modes:
- **Regular functions** (`compile_function`) - Full Python object support
- **Generators/Coroutines** (`compile_generator`) - State machine with yield/await
- **Integer mode** (`compile_int_function`) - Native int64 operations

---

### Data Loading and Storing

| Opcode | Description | Modes |
|--------|-------------|-------|
| `LOAD_FAST` | Push local variable onto stack | All |
| `STORE_FAST` | Store TOS into local variable | All |
| `LOAD_CONST` | Push constant onto stack | All |
| `LOAD_GLOBAL` | Load global variable (with optional NULL push) | All |
| `STORE_GLOBAL` | Store to global variable | Regular, Generator |
| `LOAD_ATTR` | Get attribute from object (handles method calls) | All |
| `STORE_ATTR` | Set attribute on object | Regular, Generator |
| `LOAD_SUPER_ATTR` | Load attribute from super() | Regular, Generator |
| `LOAD_DEREF` | Load from closure cell | Regular, Generator |
| `STORE_DEREF` | Store to closure cell | Regular, Generator |
| `LOAD_CLOSURE` | Push closure cell for nested function | Regular, Generator |
| `LOAD_LOCALS` | Push locals dict (annotation scopes) | Regular, Generator |
| `LOAD_FROM_DICT_OR_DEREF` | Load from dict or closure | Regular, Generator |
| `LOAD_FROM_DICT_OR_GLOBALS` | Load from dict or globals | Regular, Generator |

---

### Binary Operations

| Opcode | Description | Modes |
|--------|-------------|-------|
| `BINARY_OP` | All binary operators (see below) | All |
| `BINARY_SUBSCR` | `container[key]` | Regular, Generator |
| `BINARY_SLICE` | `container[start:end]` | Regular, Generator |

**BINARY_OP variants:**
- `0`: ADD (`+`)
- `1`: AND (`&`)
- `2`: FLOOR_DIVIDE (`//`)
- `3`: LSHIFT (`<<`)
- `4`: MATRIX_MULTIPLY (`@`)
- `5`: MULTIPLY (`*`)
- `6`: REMAINDER (`%`)
- `7`: OR (`|`)
- `8`: POWER (`**`)
- `9`: RSHIFT (`>>`)
- `10`: SUBTRACT (`-`)
- `11`: TRUE_DIVIDE (`/`)
- `12`: XOR (`^`)
- `13+`: In-place variants (`+=`, `-=`, etc.)

---

### Unary Operations

| Opcode | Description | Modes |
|--------|-------------|-------|
| `UNARY_NEGATIVE` | `-STACK[-1]` | All |
| `UNARY_NOT` | `not STACK[-1]` (requires bool in 3.13) | Regular, Generator |
| `UNARY_INVERT` | `~STACK[-1]` | Regular, Generator |
| `TO_BOOL` | `bool(STACK[-1])` | Regular, Generator |

---

### Comparison Operations

| Opcode | Description | Modes |
|--------|-------------|-------|
| `COMPARE_OP` | `<`, `<=`, `==`, `!=`, `>`, `>=` | All |
| `IS_OP` | `is` / `is not` | Regular, Generator |
| `CONTAINS_OP` | `in` / `not in` | Regular, Generator |

---

### Control Flow

| Opcode | Description | Modes |
|--------|-------------|-------|
| `JUMP_FORWARD` | Unconditional forward jump | All |
| `JUMP_BACKWARD` | Unconditional backward jump (with interrupt check) | All |
| `JUMP_BACKWARD_NO_INTERRUPT` | Backward jump without interrupt check | Regular, Generator |
| `POP_JUMP_IF_FALSE` | Pop and jump if false (requires bool) | All |
| `POP_JUMP_IF_TRUE` | Pop and jump if true (requires bool) | All |
| `POP_JUMP_IF_NONE` | Pop and jump if None | Regular, Generator |
| `POP_JUMP_IF_NOT_NONE` | Pop and jump if not None | Regular, Generator |

---

### Iteration

| Opcode | Description | Modes |
|--------|-------------|-------|
| `GET_ITER` | `iter(STACK[-1])` | Regular, Generator |
| `FOR_ITER` | Get next from iterator, jump if exhausted | Regular, Generator |
| `END_FOR` | Clean up iterator at loop end | Regular, Generator |

---

### Collection Building

| Opcode | Description | Modes |
|--------|-------------|-------|
| `BUILD_LIST` | Create list from stack items | Regular, Generator |
| `BUILD_TUPLE` | Create tuple from stack items | Regular, Generator |
| `BUILD_MAP` | Create dict from stack key-value pairs | Regular, Generator |
| `BUILD_SET` | Create set from stack items | Regular, Generator |
| `BUILD_CONST_KEY_MAP` | Build dict with constant keys | Regular, Generator |
| `BUILD_SLICE` | Create slice object | Regular, Generator |

---

### Collection Operations

| Opcode | Description | Modes |
|--------|-------------|-------|
| `STORE_SUBSCR` | `container[key] = value` | Regular, Generator |
| `DELETE_SUBSCR` | `del container[key]` | Regular, Generator |
| `STORE_SLICE` | `container[start:end] = value` | Regular, Generator |
| `LIST_APPEND` | Append to list (comprehensions) | Regular, Generator |
| `SET_ADD` | Add to set (comprehensions) | Regular, Generator |
| `MAP_ADD` | Add key-value to dict (comprehensions) | Regular, Generator |
| `LIST_EXTEND` | `list.extend(seq)` | Regular, Generator |
| `SET_UPDATE` | `set.update(seq)` | Regular, Generator |
| `DICT_UPDATE` | `dict.update(mapping)` | Regular, Generator |
| `DICT_MERGE` | Merge dict (raises on duplicates) | Regular, Generator |

---

### Unpacking

| Opcode | Description | Modes |
|--------|-------------|-------|
| `UNPACK_SEQUENCE` | Unpack iterable to exact count | Regular, Generator |
| `UNPACK_EX` | Unpack with starred target `a, *b, c = x` | Regular, Generator |

---

### Function Calls

| Opcode | Description | Modes |
|--------|-------------|-------|
| `CALL` | Call function with positional args | Regular, Generator |
| `CALL_KW` | Call with keyword arguments | Regular, Generator |
| `CALL_FUNCTION_EX` | Call with `*args, **kwargs` | Regular, Generator |
| `MAKE_FUNCTION` | Create function object | Regular, Generator |
| `SET_FUNCTION_ATTRIBUTE` | Set function attribute (defaults, etc.) | Regular, Generator |
| `LOAD_BUILD_CLASS` | Load `builtins.__build_class__` | Regular, Generator |

---

### Closures

| Opcode | Description | Modes |
|--------|-------------|-------|
| `MAKE_CELL` | Create cell for closure variable | Regular, Generator |
| `COPY_FREE_VARS` | Copy free variables to frame | Regular, Generator |

---

### Imports

| Opcode | Description | Modes |
|--------|-------------|-------|
| `IMPORT_NAME` | Import module | Regular, Generator |
| `IMPORT_FROM` | Import name from module | Regular, Generator |

---

### Exception Handling

| Opcode | Description | Modes |
|--------|-------------|-------|
| `PUSH_EXC_INFO` | Push exception info for handler | Regular, Generator |
| `POP_EXCEPT` | Pop exception state | Regular, Generator |
| `CHECK_EXC_MATCH` | Test if exception matches type | Regular, Generator |
| `RAISE_VARARGS` | Raise exception | Regular, Generator |
| `RERAISE` | Re-raise current exception | Regular, Generator |
| `CHECK_EG_MATCH` | Exception group matching (`except*`) | Regular, Generator |

---

### Context Managers

| Opcode | Description | Modes |
|--------|-------------|-------|
| `BEFORE_WITH` | Set up `with` block | Regular, Generator |
| `WITH_EXCEPT_START` | Call `__exit__` on exception | Regular, Generator |
| `BEFORE_ASYNC_WITH` | Set up `async with` block | Regular, Generator |

---

### Stack Manipulation

| Opcode | Description | Modes |
|--------|-------------|-------|
| `POP_TOP` | Pop and discard TOS | All |
| `COPY` | Duplicate item at position i | Regular, Generator |
| `SWAP` | Swap TOS with item at position i | Regular, Generator |
| `PUSH_NULL` | Push NULL (for call convention) | Regular, Generator |

---

### Generators and Coroutines

| Opcode | Description | Modes |
|--------|-------------|-------|
| `RESUME` | Resume execution (no-op, tracing) | All |
| `RETURN_VALUE` | Return TOS to caller | All |
| `RETURN_CONST` | Return constant to caller | Regular, Generator |
| `RETURN_GENERATOR` | Create generator from frame | Generator |
| `YIELD_VALUE` | Yield value from generator | Generator |

---

### Async/Await

| Opcode | Description | Modes |
|--------|-------------|-------|
| `GET_AWAITABLE` | Get awaitable for `await` | Generator |
| `SEND` | Send value to sub-iterator | Generator |
| `END_SEND` | Clean up after send | Generator |
| `CLEANUP_THROW` | Handle throw/close exception | Generator |

---

### Intrinsics

| Opcode | Description | Modes |
|--------|-------------|-------|
| `CALL_INTRINSIC_1` | Single-arg intrinsic (TYPEVAR, PARAMSPEC, etc.) | Regular, Generator |
| `CALL_INTRINSIC_2` | Two-arg intrinsic | Regular, Generator |

---

### Miscellaneous

| Opcode | Description | Modes |
|--------|-------------|-------|
| `NOP` | No operation | All |
| `CACHE` | Space for inline caching (skipped) | All |
| `EXIT_INIT_CHECK` | Verify `__init__` returned None | Regular, Generator |
| `SETUP_ANNOTATIONS` | Initialize `__annotations__` | Regular, Generator |

---

## Missing Opcodes

These opcodes are not yet implemented:

### Pattern Matching (Python 3.10+)
- `MATCH_MAPPING` - Check if mapping
- `MATCH_SEQUENCE` - Check if sequence
- `MATCH_KEYS` - Match mapping keys
- `MATCH_CLASS` - Match class pattern
- `GET_LEN` - Get length for pattern
- `COPY_DICT_WITHOUT_KEYS` - Dict pattern helper

### Async Iteration
- `GET_AITER` - `__aiter__()`
- `GET_ANEXT` - `__anext__()`
- `END_ASYNC_FOR` - Terminate async for

### Other
- `LOAD_NAME` / `STORE_NAME` / `DELETE_NAME` - Module scope names
- `DELETE_FAST` - Delete local variable
- `DELETE_DEREF` - Delete closure variable
- `DELETE_ATTR` - Delete attribute
- `DELETE_GLOBAL` - Delete global
- `LOAD_ASSERTION_ERROR` - Push AssertionError
- `GET_YIELD_FROM_ITER` - Yield from iterator
- `BUILD_STRING` - Concatenate strings
- `FORMAT_VALUE` - Format string value
- `PRINT_EXPR` - REPL print

---

## Implementation Details

### Architecture

JustJIT compiles Python bytecode to native machine code via LLVM:

1. **Stack simulation** - `std::vector<llvm::Value*>` tracks operand stack
2. **Python C API** - Objects manipulated via Py* functions
3. **Reference counting** - Explicit Py_INCREF/Py_DECREF in generated code
4. **Control flow** - Basic blocks for jumps, phi nodes for merging

### Python 3.13 Specifics

- Jump targets are relative offsets
- `CALL` argc excludes self parameter
- `POP_JUMP_IF_*` requires exact bool operand
- `LOAD_ATTR` low bit indicates method loading
- `LOAD_GLOBAL` low bit indicates NULL push for calls

### Three Compilation Modes

1. **Regular (`compile_function`)** - General Python functions
2. **Generator (`compile_generator`)** - Generator/coroutine state machines
3. **Integer (`compile_int_function`)** - Pure int64 arithmetic

---

## Summary Statistics

- **Total implemented:** 65+ opcodes
- **Coverage:** ~85% of commonly-used opcodes
- **Missing:** Pattern matching, async iteration, some delete operations

---

*Last updated: December 25, 2025*
*Python version: 3.13*
*JustJIT version: 0.2.0*
