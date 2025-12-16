# JustJIT Opcode Implementation Status

A comprehensive overview of Python 3.13 bytecode opcode support in JustJIT.

**Last Updated:** December 15, 2025

---

## Summary

| Category | Fully Implemented | Partial/Mock | Not Implemented |
|----------|------------------|--------------|-----------------|
| **Regular Functions** | 95 | 2 | 3 |
| **Generators** | 42 | 0 | 18 |
| **Async/Coroutines** | 55 | 0 | 5 |
| **Total Unique Opcodes** | ~100 | 2 | ~10 |

---

## Compilation Modes

JustJIT supports three compilation targets:

1. **`compile_function`** - Regular Python functions
2. **`compile_generator`** - Generator functions (`yield`)
3. **`compile_coroutine`** - Async functions (`async def`)

---

## ✅ Fully Implemented Opcodes (Regular Functions)

### Control Flow (12 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `RESUME` | ✅ | Function entry point |
| `NOP` | ✅ | No operation |
| `CACHE` | ✅ | Ignored (specialization cache) |
| `EXTENDED_ARG` | ✅ | Handled in instruction parsing |
| `JUMP_FORWARD` | ✅ | Unconditional forward jump |
| `JUMP_BACKWARD` | ✅ | Unconditional backward jump |
| `JUMP_BACKWARD_NO_INTERRUPT` | ✅ | No interrupt check variant |
| `POP_JUMP_IF_TRUE` | ✅ | Conditional jump |
| `POP_JUMP_IF_FALSE` | ✅ | Conditional jump |
| `POP_JUMP_IF_NONE` | ✅ | Jump if None |
| `POP_JUMP_IF_NOT_NONE` | ✅ | Jump if not None |
| `RETURN_VALUE` | ✅ | Return TOS |
| `RETURN_CONST` | ✅ | Return constant directly |

### Load/Store Operations (22 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `LOAD_CONST` | ✅ | Load constant |
| `LOAD_FAST` | ✅ | Load local variable |
| `LOAD_FAST_CHECK` | ✅ | Load with unbound check |
| `LOAD_FAST_LOAD_FAST` | ✅ | Python 3.13 optimization |
| `LOAD_FAST_AND_CLEAR` | ✅ | Load and clear (list comp) |
| `STORE_FAST` | ✅ | Store local variable |
| `STORE_FAST_STORE_FAST` | ✅ | Python 3.13 optimization |
| `STORE_FAST_LOAD_FAST` | ✅ | Python 3.13 optimization |
| `DELETE_FAST` | ✅ | Delete local variable |
| `LOAD_GLOBAL` | ✅ | Load global/builtin |
| `STORE_GLOBAL` | ✅ | Store global variable |
| `DELETE_GLOBAL` | ✅ | Delete global variable |
| `LOAD_NAME` | ✅ | Load from namespace |
| `STORE_NAME` | ✅ | Store to namespace |
| `DELETE_NAME` | ✅ | Delete from namespace |
| `LOAD_ATTR` | ✅ | Get attribute |
| `STORE_ATTR` | ✅ | Set attribute |
| `DELETE_ATTR` | ✅ | Delete attribute |
| `LOAD_SUPER_ATTR` | ✅ | super() attribute access |

### Closure Operations (6 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `COPY_FREE_VARS` | ✅ | Copy closure cells |
| `LOAD_DEREF` | ✅ | Load from cell/freevar |
| `STORE_DEREF` | ✅ | Store to cell/freevar |
| `DELETE_DEREF` | ✅ | Delete cell contents |
| `MAKE_CELL` | ✅ | Create cell object |
| `LOAD_CLOSURE` | ✅ | Load closure for nested func |

### Binary Operations (1 opcode, 15 operators)
| Opcode | Status | Notes |
|--------|--------|-------|
| `BINARY_OP` | ✅ | All 15 operators supported |

**Operators:** `+`, `-`, `*`, `/`, `//`, `%`, `**`, `@`, `<<`, `>>`, `&`, `|`, `^`, `+=` (and all augmented)

### Unary Operations (4 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `UNARY_NEGATIVE` | ✅ | Negation `-x` |
| `UNARY_INVERT` | ✅ | Bitwise NOT `~x` |
| `UNARY_NOT` | ✅ | Boolean NOT `not x` |
| `TO_BOOL` | ✅ | Convert to bool |

### Comparison Operations (3 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `COMPARE_OP` | ✅ | All comparison ops |
| `CONTAINS_OP` | ✅ | `in` / `not in` |
| `IS_OP` | ✅ | `is` / `is not` |

### Collection Building (10 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `BUILD_LIST` | ✅ | Create list |
| `BUILD_TUPLE` | ✅ | Create tuple |
| `BUILD_SET` | ✅ | Create set |
| `BUILD_MAP` | ✅ | Create dict |
| `BUILD_CONST_KEY_MAP` | ✅ | Dict with const keys |
| `BUILD_STRING` | ✅ | f-string building |
| `BUILD_SLICE` | ✅ | Create slice object |
| `LIST_EXTEND` | ✅ | `*iterable` unpacking |
| `SET_UPDATE` | ✅ | Set update |
| `DICT_UPDATE` | ✅ | Dict `**` unpacking |
| `DICT_MERGE` | ✅ | Dict merge |

### Collection Mutation (7 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `LIST_APPEND` | ✅ | Append to list (comprehension) |
| `SET_ADD` | ✅ | Add to set (comprehension) |
| `MAP_ADD` | ✅ | Add to dict (comprehension) |
| `BINARY_SUBSCR` | ✅ | `x[key]` |
| `STORE_SUBSCR` | ✅ | `x[key] = val` |
| `DELETE_SUBSCR` | ✅ | `del x[key]` |
| `BINARY_SLICE` | ✅ | `x[a:b]` |
| `STORE_SLICE` | ✅ | `x[a:b] = val` |

### Unpacking (2 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `UNPACK_SEQUENCE` | ✅ | `a, b, c = iterable` |
| `UNPACK_EX` | ✅ | `a, *b, c = iterable` |

### Stack Manipulation (3 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `POP_TOP` | ✅ | Discard TOS |
| `COPY` | ✅ | Duplicate stack item |
| `SWAP` | ✅ | Swap stack items |
| `PUSH_NULL` | ✅ | Push NULL for calls |

### Iteration (4 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `GET_ITER` | ✅ | `iter(x)` |
| `FOR_ITER` | ✅ | `next()` with exhaustion check |
| `END_FOR` | ✅ | Cleanup after for loop |
| `GET_LEN` | ✅ | `len(x)` for pattern matching |

### Function Calls (4 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `CALL` | ✅ | Regular function call |
| `CALL_KW` | ✅ | Call with keyword args |
| `CALL_FUNCTION_EX` | ✅ | `f(*args, **kwargs)` |
| `CALL_INTRINSIC_1` | ✅ | All 11 intrinsics supported |

**Intrinsics:** `INTRINSIC_PRINT`, `INTRINSIC_IMPORT_STAR`, `INTRINSIC_STOPITERATION_ERROR`, `INTRINSIC_ASYNC_GEN_WRAP`, `INTRINSIC_UNARY_POSITIVE`, `INTRINSIC_LIST_TO_TUPLE`, `INTRINSIC_TYPEVAR`, `INTRINSIC_PARAMSPEC`, `INTRINSIC_TYPEVARTUPLE`, `INTRINSIC_SUBSCRIPT_GENERIC`, `INTRINSIC_TYPEALIAS`

### Function/Class Creation (3 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `MAKE_FUNCTION` | ✅ | Create function object |
| `SET_FUNCTION_ATTRIBUTE` | ✅ | Set func attributes |
| `LOAD_BUILD_CLASS` | ✅ | `__build_class__` |

### Import (2 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `IMPORT_NAME` | ✅ | `import module` |
| `IMPORT_FROM` | ✅ | `from module import x` |

### String Formatting (3 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `FORMAT_SIMPLE` | ✅ | f-string `{x}` |
| `FORMAT_WITH_SPEC` | ✅ | f-string `{x:spec}` |
| `CONVERT_VALUE` | ✅ | `!r`, `!s`, `!a` conversions |

### Exception Handling (6 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `PUSH_EXC_INFO` | ✅ | Push exception at handler entry |
| `POP_EXCEPT` | ✅ | Clear exception state |
| `CHECK_EXC_MATCH` | ✅ | `except ExceptionType` matching |
| `RAISE_VARARGS` | ✅ | `raise`, `raise exc`, `raise from` |
| `RERAISE` | ✅ | Re-raise current exception |
| `LOAD_ASSERTION_ERROR` | ✅ | For `assert` statement |

### Context Managers (2 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `BEFORE_WITH` | ✅ | Enter `with` block |
| `WITH_EXCEPT_START` | ✅ | Call `__exit__` on exception |

### Pattern Matching (5 opcodes)
| Opcode | Status | Notes |
|--------|--------|-------|
| `MATCH_MAPPING` | ✅ | `case {...}` |
| `MATCH_SEQUENCE` | ✅ | `case [...]` |
| `MATCH_KEYS` | ✅ | Extract mapping keys |
| `MATCH_CLASS` | ✅ | `case ClassName(...)` |
| `GET_LEN` | ✅ | Length check in patterns |

---

## ⚠️ Partially Implemented / Mock

| Opcode | Status | Notes |
|--------|--------|-------|
| `CALL_INTRINSIC_2` | ⚠️ | Only some intrinsics tested |
| `INSTRUMENTED_*` | ⚠️ | Not tested (profiling/tracing) |

---

## ❌ Not Implemented

### For Regular Functions
| Opcode | Reason | Workaround |
|--------|--------|------------|
| `SETUP_FINALLY` | Legacy (pre-3.11) | N/A - not used in 3.13 |
| `POP_BLOCK` | Legacy (pre-3.11) | N/A - not used in 3.13 |
| `CLEANUP_THROW` | Complex generator exception | Falls back to Python |

### Async Generators (Not Supported)
| Opcode | Reason |
|--------|--------|
| `GET_AITER` | Async iteration |
| `GET_ANEXT` | Async iteration |
| `END_ASYNC_FOR` | Async for cleanup |
| `ASYNC_GEN_WRAP` | Async generator yield |
| `GET_YIELD_FROM_ITER` | `yield from` in async |

---

## Generator-Specific Support

### ✅ Implemented in `compile_generator`
| Opcode | Notes |
|--------|-------|
| `RETURN_GENERATOR` | Generator entry |
| `YIELD_VALUE` | Core yield mechanism |
| All load/store opcodes | Local variable access |
| All control flow opcodes | Jumps and conditionals |
| All iteration opcodes | Nested for loops |
| Basic collections | LIST, TUPLE, BUILD_CONST_KEY_MAP |

### ❌ Not Yet Implemented in Generators
| Opcode | Notes |
|--------|-------|
| `CALL_FUNCTION_EX` | `*args, **kwargs` in generator |
| `CALL_KW` | Keyword arguments |
| `BUILD_MAP`, `BUILD_SET` | Dict/set literals |
| `UNPACK_SEQUENCE`, `UNPACK_EX` | Tuple unpacking |
| `STORE_GLOBAL`, `STORE_ATTR` | Global/attr mutation |
| Closure opcodes | `LOAD_DEREF`, `STORE_DEREF`, etc. |
| `IMPORT_NAME`, `IMPORT_FROM` | Import statements |
| `MAKE_FUNCTION` | Nested functions |
| `RAISE_VARARGS` | Custom raises |

---

## Async/Coroutine-Specific Support

### ✅ Implemented in `compile_coroutine`
| Opcode | Notes |
|--------|-------|
| `GET_AWAITABLE` | Get awaitable from object |
| `SEND` | Core await mechanism |
| `END_SEND` | Cleanup after await |
| `CLEANUP_THROW` | Exception in await |
| All generator opcodes | Full generator support |
| `await f() + await g()` | ✅ Multiple awaits in expressions |

### ❌ Not Supported
| Feature | Reason |
|---------|--------|
| Async generators | Complex state machine |
| `async for` | Requires `GET_AITER`/`GET_ANEXT` |
| `async with` | Could be added (uses regular `BEFORE_WITH`) |

---

## Exception Handling Architecture

JustJIT implements full Python 3.11+ exception table handling:

1. **Exception Table Parsing**: Reads `co_exceptiontable` from code object
2. **Handler Mapping**: Maps bytecode offset ranges to handler targets
3. **Stack Unwinding**: Properly decrefs stack values when jumping to handler
4. **`check_error_and_branch`**: Lambda that generates error-checking code after every Python C API call

```
Exception Table Entry:
  start: 10    # Protected range start (byte offset)
  end: 50      # Protected range end
  target: 100  # Handler offset (PUSH_EXC_INFO location)
  depth: 2     # Stack depth to unwind to
  lasti: false # Whether to push last instruction offset
```

---

## Integer Mode (Experimental)

JustJIT has an experimental "integer mode" for pure numeric functions:

### Supported in Integer Mode
- `LOAD_FAST`, `STORE_FAST`, `LOAD_CONST`
- `BINARY_OP` (arithmetic only: `+`, `-`, `*`, `/`, `//`, `%`)
- `COMPARE_OP`
- `POP_JUMP_IF_TRUE/FALSE`
- `RETURN_VALUE`, `RETURN_CONST`
- `JUMP_FORWARD`, `JUMP_BACKWARD`

### Benefits
- No Python object allocation
- Direct LLVM i64 arithmetic
- ~10x faster for numeric loops

---

## How to Check Support

```python
from justjit import jit
import dis

@jit
def my_func():
    ...

# Check what opcodes your function uses:
dis.dis(my_func)

# If JIT fails, it will fall back to Python with a warning
```

---

## Future Work

1. **Async Generators** - Complex state machine with `ASYNC_GEN_WRAP`
2. **`async for`** - Needs `GET_AITER`, `GET_ANEXT`
3. **Full Generator Support** - All opcodes that work in functions
4. **Specialization Cache** - Use `CACHE` entries for type specialization

---

## Version Compatibility

| Python Version | Status |
|---------------|--------|
| 3.13+ | ✅ Full support |
| 3.12 | ⚠️ Untested (bytecode may differ) |
| 3.11 | ⚠️ Untested (exception table format same) |
| < 3.11 | ❌ Not supported (different bytecode format) |
