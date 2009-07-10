
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "commandQueue.h"

#ifdef GLX
#  include "glXMessagePump.h"
#endif
#ifdef WGL
#  include "wglMessagePump.h"
#endif
#ifdef AGL
#  include "aglMessagePump.h"
#endif

using namespace std;

namespace eq
{
CommandQueue::CommandQueue()
        : _messagePump( 0 )
        , _windowSystem( WINDOW_SYSTEM_NONE )
{
}

CommandQueue::~CommandQueue()
{
    delete _messagePump;
    _messagePump = 0;
}

void CommandQueue::setWindowSystem( const WindowSystem windowSystem )
{
    if( _windowSystem == windowSystem )
        return;

    EQASSERTINFO( _windowSystem == WINDOW_SYSTEM_NONE, 
                  "Can't switch window system from " << _windowSystem << " to "
                  << windowSystem );
    EQASSERT( !_windowSystem );

    _windowSystem = windowSystem;

    switch( windowSystem )
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX:
            _messagePump = new GLXMessagePump();
            break;
#endif

#ifdef WGL
        case WINDOW_SYSTEM_WGL:
            _messagePump = new WGLMessagePump();
            break;
#endif

#ifdef AGL
        case WINDOW_SYSTEM_AGL:
            _messagePump = new AGLMessagePump();
            break;
#endif

        default:
            EQUNREACHABLE;
    }
}

void CommandQueue::push(net::Command& inCommand)
{
    net::CommandQueue::push(inCommand);
    if( _messagePump )
        _messagePump->postWakeup();
}

void CommandQueue::pushFront(net::Command& inCommand)
{
    net::CommandQueue::pushFront(inCommand);
    if( _messagePump )
        _messagePump->postWakeup();
}

void CommandQueue::wakeup()
{
    net::CommandQueue::wakeup();
    if( _messagePump )
        _messagePump->postWakeup();
}

net::Command* CommandQueue::pop()
{
    while( true )
    {
        if( _messagePump )
            _messagePump->dispatchAll(); // non-blocking

        // Poll for a command
        if( !isEmpty( ))
            return net::CommandQueue::pop();

        if( _messagePump )
            _messagePump->dispatchOne(); // blocking - push will send wakeup
        else
            return net::CommandQueue::pop();
    }
}

net::Command* CommandQueue::tryPop()
{
    if( _messagePump )
        _messagePump->dispatchAll(); // non-blocking

    return net::CommandQueue::tryPop();
}

void CommandQueue::flush()
{
    if( _messagePump )
        _messagePump->dispatchDone();

    net::CommandQueue::flush();
}

}
