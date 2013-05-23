
/* Copyright (c) 2009-2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__GPU_ASYNC_LOADER_BASE_H
#define MASS_VOL__GPU_ASYNC_LOADER_BASE_H

#include <msv/types/nonCopyable.h>

#include <lunchbox/thread.h>

struct GLEWContextStruct;
typedef struct GLEWContextStruct GLEWContext;

namespace eq {
    class SystemWindow;
    class ComputeContext;
}

namespace massVolVis
{

class Window;

/**
 *  Asynchronous fetching thread. Creates and supplies new textures to the main rendering pipe.
 */
class GPUAsyncLoaderBase : public lunchbox::Thread, private NonCopyable
{
public:
    GPUAsyncLoaderBase( Window* wnd );
    virtual ~GPUAsyncLoaderBase();

    /** 
     *  Do stuff like unlocking main thread, but not data initialization.
     *  Called before runLocal() in any case, even if initialization fails.
     */
    virtual void onInit() {};

    /** 
     *  Main processing loop.
     *  If initialization fails it will not be called.
     */
    virtual void runLocal() {}; 

    /** 
     *  Delete object manager, etc. (called after runLocal() exits ).
     *  If initialization fails it will not be called.
     */
    virtual void cleanup() {}; 

    /** 
     *  Initializes asyncronious windows, calles onInit(), runLocal() and cleanup().
     *  Shouldn't be modified.
     */
    virtual void run();

    const GLEWContext* glewGetContext() const;

    Window*           getWindow()              { return _wnd;                 };
    eq::SystemWindow* getSharedContextWindow() { return _sharedContextWindow; };

private:
    Window*             _wnd;
    eq::SystemWindow*   _sharedContextWindow;
    eq::ComputeContext* _computeCtx;
};

}

#endif //MASS_VOL__GPU_ASYNC_LOADER_BASE_H
