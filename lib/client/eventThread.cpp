/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "eventThread.h"

#ifdef GLX
#  include "glXEventThread.h"
#endif

#include <eq/base/lock.h>

using namespace eq;
using namespace eqBase;
using namespace std;

EventThread* EventThread::_threads[WINDOW_SYSTEM_ALL] = { NULL };

static Lock _threadsLock;

EventThread* EventThread::get( const WindowSystem windowSystem )
{
    if( _threads[windowSystem] )
        return _threads[windowSystem];

    _threadsLock.set();
    if( !_threads[windowSystem] )
    {
        switch( windowSystem )
        {
            case WINDOW_SYSTEM_GLX:
#ifdef GLX
                _threads[windowSystem] = new GLXEventThread;
#endif
                break;
            default:
                EQERROR << "Event thread unimplemented for window system "
                        << windowSystem << endl;
                return NULL;
        }
    }
    _threadsLock.unset();

    EQASSERT( _threads[windowSystem] );
    return _threads[windowSystem];
}
