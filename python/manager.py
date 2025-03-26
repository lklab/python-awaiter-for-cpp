import asyncio
import threading

loop: asyncio.AbstractEventLoop = None
thread: threading.Thread = None
_cppmodule = None

def _event_loop(loop: asyncio.AbstractEventLoop) :
    try :
        loop.run_forever()
    except :
        pass
    finally :
        loop.close()

async def _stop_event_loop(loop: asyncio.AbstractEventLoop) :
    loop.stop()

def initialize() :
    global loop
    global _cppmodule
    if loop != None :
        return

    # start event loop
    global thread
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    thread = threading.Thread(target=_event_loop, args=(loop,))
    thread.start()

    # import c module
    import cppmodule as cppmodule # type: ignore
    _cppmodule = cppmodule

def terminate() :
    global loop
    if loop == None :
        return

    # stop event loop
    global thread
    asyncio.run_coroutine_threadsafe(_stop_event_loop(loop), loop)
    thread.join()
