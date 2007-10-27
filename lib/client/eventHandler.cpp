/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "eventHandler.h"

#ifdef GLX
#  include "glXEventThread.h"
#endif
#ifdef WGL
#  include "wglEventHandler.h"
#endif
#ifdef AGL
#  include "aglEventHandler.h"
#endif

#include "pipe.h"
#include "window.h"

#include <eq/base/lock.h>
#include <eq/base/debug.h>

using namespace eq;
using namespace eqBase;
using namespace std;

EventHandler* EventHandler::registerPipe( Pipe* pipe )
{
    switch( pipe->getWindowSystem( ))
    {
        case WINDOW_SYSTEM_GLX:
#ifdef GLX
        {
            GLXEventThread* thread = GLXEventThread::get();
            thread->registerPipe( pipe );
            return thread;
        }
#endif
            break;

        case WINDOW_SYSTEM_AGL:
        case WINDOW_SYSTEM_WGL:
            // NOP
            break;

        default:
            EQERROR << "event handling not implemented for window system " 
                    << pipe->getWindowSystem() << endl;
            break;
    }
    return 0;
}

EventHandler* EventHandler::registerWindow( Window* window )
{
    const Pipe* pipe = window->getPipe();
    if( !pipe )
    {
        EQWARN << "Can't determine window system: no parent pipe" << endl;
        return 0;
    }

    switch( pipe->getWindowSystem( ))
    {
        case WINDOW_SYSTEM_GLX:
#ifdef GLX
        {
            GLXEventThread* thread = GLXEventThread::get();
            thread->registerWindow( window );
            return thread;
        }
#endif
            break;

        case WINDOW_SYSTEM_AGL:
#ifdef AGL
        {
            AGLEventHandler* handler = AGLEventHandler::get();
            handler->registerWindow( window );
            return handler;
        }
#endif
            break;

        case WINDOW_SYSTEM_WGL:
#ifdef WGL
            return new WGLEventHandler( window );
#endif
            break;

        default:
            EQERROR << "event handling not implemented for window system " 
                    << pipe->getWindowSystem() << endl;
            break;
    }
    return 0;
}


void EventHandler::_computePointerDelta( WindowEvent &event )
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
        case WindowEvent::POINTER_BUTTON_PRESS:
        case WindowEvent::POINTER_BUTTON_RELEASE:
            if( _lastPointerEvent.type == WindowEvent::POINTER_MOTION )
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
