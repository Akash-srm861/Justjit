# Async/Coroutine JIT Implementation Research

## Summary

This document captures research gathered for implementing async/await JIT compilation in JustJit.

---

## Current Status

- ✅ **Regular generators** work with JIT (state machine approach)
- ❌ **Async functions** (`async def`) fall back to Python with warning
- ❌ **Async generators** (`async def` + `yield`) fall back to Python with warning

---

## Python 3.13 Async Bytecode Analysis

### Async Function Bytecode

```python
async def async_add(a, b):
    await asyncio.sleep(0)
    return a + b
```

Produces:
```
RESUME                   0
RETURN_GENERATOR            # Creates coroutine object from frame
POP_TOP
RESUME                   3   # Resume context = 3 (after await)

# await expression:
LOAD_GLOBAL              asyncio.sleep
PUSH_NULL
LOAD_CONST               0
CALL                     1
GET_AWAITABLE            2   # Get awaitable object (arg=2 for await expr)
LOAD_CONST               None
SEND                     4   # delta for StopIteration handler
YIELD_VALUE              1   # arg=1 indicates yield-from/await
RESUME                   3
END_SEND                    # Cleanup stack after send

# return:
LOAD_FAST_LOAD_FAST      0, 1
BINARY_OP                0
RETURN_VALUE
```

### Async Generator Bytecode

```python
async def async_gen(n):
    for i in range(n):
        await asyncio.sleep(0)
        yield i
```

Produces additional opcodes:
```
CALL_INTRINSIC_1         4   # INTRINSIC_ASYNC_GEN_WRAP - wraps yielded value
YIELD_VALUE              0   # Regular yield (arg=0)
```

---

## Key Opcodes to Implement

### Core Await Mechanism

| Opcode | Description | Stack Effect |
|--------|-------------|--------------|
| `RETURN_GENERATOR` | Creates coroutine object, sets `CO_COROUTINE` flag | Special |
| `GET_AWAITABLE(where)` | Gets awaitable via `__await__` or validates coroutine | 0 (transforms TOS) |
| `SEND(delta)` | `STACK[-1] = STACK[-2].send(STACK[-1])`, jump if StopIteration | Complex |
| `END_SEND` | `del STACK[-2]` - cleanup after send completes | -1 |
| `CLEANUP_THROW` | Exception handling during throw()/close() | Special |

### Async Generator Specific

| Opcode | Description | Stack Effect |
|--------|-------------|--------------|
| `CALL_INTRINSIC_1(4)` | INTRINSIC_ASYNC_GEN_WRAP - wraps value for async gen | 0 |
| `GET_AITER` | `STACK[-1].__aiter__()` | 0 |
| `GET_ANEXT` | `get_awaitable(STACK[-1].__anext__())` | +1 |
| `END_ASYNC_FOR` | Terminates async for loop | Special |

### Resume Context Values

The `RESUME` opcode uses these context values in arg:
- 0 = At function start
- 1 = After yield
- 2 = After yield-from
- 3 = After await

### YIELD_VALUE Argument

- `arg=0`: Regular yield
- `arg=1`: Yield from / await expression

---

## How Async Works Internally

### From Stack Overflow Research

1. **Coroutines are like generators** but use `CO_COROUTINE` flag
2. **`await` is essentially `yield from`** with awaitable validation
3. **Event loop drives execution** via `send()` and `throw()` methods
4. **Futures/Tasks communicate** with event loop through yield chain

### Key Insight from PEP 492/525

```python
# await is approximately:
result = yield from awaitable.__await__()

# Which is roughly:
_iter = awaitable.__await__()
while True:
    try:
        yielded = _iter.send(None)
        yield yielded  # Pass up to event loop
    except StopIteration as e:
        result = e.value
        break
```

### Event Loop Interaction

1. Event loop calls `coroutine.send(None)` to start/resume
2. Coroutine runs until `await` expression
3. `await` yields control back to event loop with a Future
4. Event loop registers callback on Future
5. When Future completes, event loop calls `coroutine.send(result)`
6. Process repeats until `StopIteration` is raised

---

## LLVM Coroutine Support

LLVM has built-in coroutine support via intrinsics:

### Key LLVM Intrinsics

```llvm
@llvm.coro.id      ; Coroutine identity
@llvm.coro.begin   ; Initialize coroutine frame
@llvm.coro.suspend ; Suspend point
@llvm.coro.end     ; Mark coroutine end
@llvm.coro.resume  ; Resume suspended coroutine
@llvm.coro.destroy ; Destroy coroutine frame
@llvm.coro.done    ; Check if at final suspend
```

### LLVM Coroutine Frame

LLVM automatically:
1. Analyzes def-use chains across suspend points
2. Spills live values to coroutine frame
3. Splits function into ramp, resume, and destroy parts
4. Generates switch-based state machine

### Possible Approach

Could use LLVM's coroutine passes:
1. Emit `llvm.coro.*` intrinsics at suspend points
2. Let LLVM CoroSplit pass split into resume functions
3. Store state in coroutine frame (similar to our generator state)

**OR** continue with manual state machine (like current generators):
1. Create step function with state parameter
2. Switch on state to resume at correct point
3. Save/restore locals and stack at each suspend

---

## Implementation Strategy Options

### Option A: Extend Current Generator Approach

Pros:
- Similar to existing working code
- Full control over state machine
- No dependency on LLVM coroutine passes

Cons:
- More complex for async (need to handle await chains)
- Must manually track live values across suspends

### Option B: Use LLVM Coroutine Intrinsics

Pros:
- LLVM handles frame layout and splitting
- More robust for complex control flow
- Standard approach used by C++ coroutines

Cons:
- Different from current generator implementation
- Need to integrate with Python's coroutine protocol
- May be harder to integrate with Python's event loop

### Recommended: Hybrid Approach

1. **Phase 1**: Start with manual state machine (like generators)
   - Implement `GET_AWAITABLE`, `SEND`, `END_SEND`
   - Return coroutine wrapper that integrates with asyncio

2. **Phase 2**: Consider LLVM coroutines for optimization
   - Once basic async works, explore LLVM coroutine passes
   - Could improve performance for complex async functions

---

## CPython Implementation Details

### genobject.c Key Functions

```c
// Core send implementation
static PyObject* gen_send_ex2(PyGenObject *gen, PyObject *arg, 
                              PyObject **presult, int exc, int closing);

// Close implementation
static PyObject* gen_close(PyObject *self, PyObject *args);

// Throw implementation  
static PyObject* _gen_throw(PyGenObject *gen, int close_on_genexit,
                           PyObject *typ, PyObject *val, PyObject *tb);
```

### Key Type Structures

```c
PyTypeObject PyCoro_Type = {
    "coroutine",
    .tp_as_async = &coro_as_async,  // am_await = coro_await
    ...
};

PyTypeObject PyAsyncGen_Type = {
    "async_generator",
    .tp_as_async = &async_gen_as_async,  // am_aiter, am_anext
    ...
};
```

### Frame States

```c
typedef enum _framestate {
    FRAME_CREATED = -2,    // Generator created but not started
    FRAME_SUSPENDED = -1,  // Suspended at yield
    FRAME_EXECUTING = 0,   // Currently executing
    FRAME_COMPLETED = 1,   // Finished execution
    FRAME_CLEARED = 4,     // Frame cleared
} PyFrameState;
```

---

## Required Python Wrapper

Need to create Python wrapper that:

1. **Implements coroutine protocol**
   - `send(value)` - Resume with value
   - `throw(exc)` - Inject exception
   - `close()` - Close coroutine
   - `__await__()` - Return self for await

2. **Manages state**
   - Frame state (created/suspended/executing/completed)
   - Current suspend point
   - Local variables
   - Stack values

3. **Integrates with asyncio**
   - Return awaitables that asyncio understands
   - Support `async with` and `async for`

---

## Implementation Checklist

### Phase 1: Basic Async Functions
- [ ] Implement `GET_AWAITABLE` opcode
- [ ] Implement `SEND` opcode with StopIteration handling
- [ ] Implement `END_SEND` cleanup
- [ ] Create coroutine wrapper class in Python
- [ ] Handle RESUME context=3 (after await)
- [ ] Test with simple await expressions

### Phase 2: Error Handling
- [ ] Implement `CLEANUP_THROW` opcode
- [ ] Add throw() method to wrapper
- [ ] Add close() method to wrapper
- [ ] Handle GeneratorExit properly

### Phase 3: Async Generators
- [ ] Implement `CALL_INTRINSIC_1` with ASYNC_GEN_WRAP
- [ ] Create async generator wrapper
- [ ] Implement `__aiter__`, `__anext__`
- [ ] Implement `asend()`, `athrow()`, `aclose()`

### Phase 4: Async Iteration
- [ ] Implement `GET_AITER` opcode
- [ ] Implement `GET_ANEXT` opcode
- [ ] Implement `END_ASYNC_FOR` opcode
- [ ] Test with `async for` loops

---

## References

1. **PEP 492** - Coroutines with async and await syntax
   - https://peps.python.org/pep-0492/

2. **PEP 525** - Asynchronous Generators
   - https://peps.python.org/pep-0525/

3. **Python dis module** - Bytecode instructions
   - https://docs.python.org/3.13/library/dis.html

4. **LLVM Coroutines** - LLVM coroutine lowering
   - https://llvm.org/docs/Coroutines.html

5. **CPython genobject.c** - Generator/coroutine implementation
   - https://github.com/python/cpython/blob/main/Objects/genobject.c

6. **Stack Overflow** - How asyncio actually works
   - https://stackoverflow.com/questions/49005651/
