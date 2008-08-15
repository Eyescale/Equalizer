/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "eventHandler.h"

#ifdef GLX
#  include "glXEventHandler.h"
#endif

#include "pipe.h"
#include "window.h"

#include <eq/base/lock.h>
#include <eq/base/debug.h>

using namespace eq::base;
using namespace std;

namespace eq
{
EventHandler* EventHandler::registerPipe( Pipe* pipe )
{
    switch( pipe->getWindowSystem( ))
    {
        case WINDOW_SYSTEM_GLX:
#ifdef GLX
            return new GLXEventHandler( pipe );
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

void EventHandler::_computePointerDelta( WindowEvent &event )
{
    if( _lastPointerEvent.window != event.window )
    {
        event.data.pointerEvent.dx = 0;
        event.data.pointerEvent.dy = 0;
        _lastPointerEvent = event;
        return;
    }

    switch( event.data.type )
    {
        case Event::POINTER_BUTTON_PRESS:
        case Event::POINTER_BUTTON_RELEASE:
            if( _lastPointerEvent.data.type == Event::POINTER_MOTION )
            {
                event.data.pointerEvent.dx =
                    _lastPointerEvent.data.pointerEvent.dx;
                event.data.pointerEvent.dy =
                    _lastPointerEvent.data.pointerEvent.dy;
                break;
            }
            // fall through

        default:
            event.data.pointerEvent.dx = event.data.pointerEvent.x -
                                         _lastPointerEvent.data.pointerEvent.x;
            event.data.pointerEvent.dy = event.data.pointerEvent.y -
                                         _lastPointerEvent.data.pointerEvent.y;
    }
    _lastPointerEvent = event;
}

void EventHandler::_getRenderContext( WindowEvent& event )
{
    const int32_t x = event.data.pointerEvent.x;
    const int32_t y = event.data.pointerEvent.y;

    const RenderContext* context = event.window->getRenderContext( x, y );
    if( context )
        event.data.context = *context;
    else
        EQINFO << "No rendering context for pointer event on " << x << ", " 
               << y << endl;
}

}
