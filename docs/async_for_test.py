"""Minimal test to verify compile_generator is called and debug output appears."""
import sys
import os

# Add LLVM DLLs to path  
llvm_bin = r"C:\Users\vetri\llvm-project\build\Release\bin"
if os.path.exists(llvm_bin):
    os.add_dll_directory(llvm_bin)

import asyncio
import justjit

print("===== Starting minimal async for test =====", file=sys.stderr, flush=True)

class AsyncRange:
    def __init__(self, n):
        self.n = n
        self.i = 0
    
    def __aiter__(self):
        return self
    
    async def __anext__(self):
        if self.i >= self.n:
            raise StopAsyncIteration
        i = self.i
        self.i += 1
        return i

print("===== About to decorate function =====", file=sys.stderr, flush=True)

@justjit.jit
async def sum_async_range(n):
    total = 0
    async for i in AsyncRange(n):
        total += i
    return total

print("===== Function decorated, about to call =====", file=sys.stderr, flush=True)

try:
    result = asyncio.run(sum_async_range(3))
    print(f"Result: {result}")
except Exception as e:
    print(f"Error: {e}", file=sys.stderr, flush=True)
    import traceback
    traceback.print_exc()

print("===== Test complete =====", file=sys.stderr, flush=True)
