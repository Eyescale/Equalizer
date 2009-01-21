
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

void EventHandler::_computePointerDelta( const Window* window, Event &event )
{
    if( _lastEventWindow != window )
    {
        event.pointer.dx  = 0;
        event.pointer.dy  = 0;
        _lastPointerEvent = event;
        _lastEventWindow  = window;
        return;
    }

    switch( event.type )
    {
        case Event::POINTER_BUTTON_PRESS:
        case Event::POINTER_BUTTON_RELEASE:
            if( _lastPointerEvent.type == Event::POINTER_MOTION )
            {
                event.pointer.dx = _lastPointerEvent.pointer.dx;
                event.pointer.dy = _lastPointerEvent.pointer.dy;
                break;
            }
            // fall through

        default:
            event.pointer.dx = event.pointer.x - _lastPointerEvent.pointer.x;
            event.pointer.dy = event.pointer.y - _lastPointerEvent.pointer.y;
    }
    _lastPointerEvent = event;
}

void EventHandler::_getRenderContext( const Window* window, Event& event )
{
    const int32_t x = event.pointer.x;
    const int32_t y = event.pointer.y;

    const RenderContext* context = window->getRenderContext( x, y );
    if( context )
        event.context = *context;
    else
        EQINFO << "No rendering context for pointer event at " << x << ", " 
               << y << endl;
}

}
