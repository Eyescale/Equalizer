/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "eventThread.h"

#ifdef GLX
#  include "glXEventThread.h"
#endif
#ifdef WGL
#  include "WGLEventHandler.h"
#endif

#include <eq/base/lock.h>
#include <eq/base/debug.h>

using namespace eq;
using namespace eqBase;
using namespace std;

EventThread* EventThread::_handlers[WINDOW_SYSTEM_ALL] = { NULL };

static Lock _handlersLock;

EventThread* EventThread::get( const WindowSystem windowSystem )
{
    _handlersLock.set();
    if( !_handlers[windowSystem] )
    {
        switch( windowSystem )
        {
            case WINDOW_SYSTEM_GLX:
#ifdef GLX
                _handlers[windowSystem] = new GLXEventThread;
#endif
                break;

            case WINDOW_SYSTEM_WGL:
#ifdef WGL
                _handlers[windowSystem] = new WGLEventHandler;
#endif
                break;

            default:
                break;
        }
    }
    _handlersLock.unset();
    
    if( !_handlers[windowSystem] )
        EQERROR << "Event thread unimplemented for window system "
                << windowSystem << endl;

    return _handlers[windowSystem];
}

void EventThread::_computePointerDelta( WindowEvent &event )
{
    if( _lastPointerEvent.window != event.window )
    {
        event.pointerEvent.dx = 0;
        event.pointerEvent.dy = 0;
        _lastPointerEvent = event;
        return;
    }

    switch( event.type )
    {
        case WindowEvent::TYPE_POINTER_BUTTON_PRESS:
        case WindowEvent::TYPE_POINTER_BUTTON_RELEASE:
            if( _lastPointerEvent.type == WindowEvent::TYPE_POINTER_MOTION )
            {
                event.pointerEvent.dx = _lastPointerEvent.pointerEvent.dx;
                event.pointerEvent.dy = _lastPointerEvent.pointerEvent.dy;
                break;
            }
            // fall through

        default:
            event.pointerEvent.dx = 
                event.pointerEvent.x - _lastPointerEvent.pointerEvent.x;
            event.pointerEvent.dy = 
                event.pointerEvent.y - _lastPointerEvent.pointerEvent.y;
    }
    _lastPointerEvent = event;
}
