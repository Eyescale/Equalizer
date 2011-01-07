
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2011, Cedric Stalder <cedric.stalder@gmail.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "eventHandler.h"

#ifdef GLX
#  include "glXEventHandler.h"
#endif

#include "pipe.h"
#include "window.h"

#include <co/base/lock.h>
#include <co/base/debug.h>

using namespace co::base;
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
        case Event::WINDOW_POINTER_BUTTON_PRESS:
        case Event::WINDOW_POINTER_BUTTON_RELEASE:
            if( _lastPointerEvent.type == Event::WINDOW_POINTER_MOTION )
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

    if( !window->getRenderContext( x, y, event.context ))
        EQVERB << "No rendering context for pointer event at " << x << ", " 
               << y << endl;
}

}
