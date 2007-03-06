/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "eventHandler.h"

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
