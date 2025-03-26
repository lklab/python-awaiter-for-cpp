import asyncio
import random

from python.manager import loop, _cppmodule

async def _example_func(rqid: int, arg0: int, arg1: str, arg2: float) :
    await asyncio.sleep(random.uniform(5.0, 15.0))

    global _cppmodule
    _cppmodule.cpp_callback(rqid, f'python: rqid={rqid}, arg0={arg0}, arg1={arg1}, arg2={arg2}')

def example_func(rqid: int, arg0: int, arg1: str, arg2: float) :
    global loop
    asyncio.run_coroutine_threadsafe(_example_func(rqid, arg0, arg1, arg2), loop)
