
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "messagePump.h"

#include <lunchbox/debug.h>
#include <lunchbox/log.h>
#include <lunchbox/os.h>

namespace eq
{
namespace wgl
{
MessagePump::MessagePump()
        : _win32ThreadID( 0 )
{
}

void MessagePump::postWakeup()
{
    if( !_win32ThreadID )
    {
        LBWARN << "Receiver thread not waiting?" << std::endl;
        return;
    }

    PostThreadMessage( _win32ThreadID, WM_APP, 0, 0 ); // Wake up pop()
}

void MessagePump::_initReceiverQueue()
{
    if( !_win32ThreadID )
    {
        MSG msg;
        PeekMessage( &msg, 0, WM_USER, WM_USER, PM_NOREMOVE );
        _win32ThreadID = GetCurrentThreadId();
        LBASSERT( _win32ThreadID );
    }
    LBASSERTINFO( _win32ThreadID == GetCurrentThreadId(),
                  "wgl::MessagePump::pop() called from two different threads" );
}

void MessagePump::dispatchOne( const uint32_t timeout )
{
    _initReceiverQueue();

    // TODO timeout implementation using MsgWaitForMultipleObjects
    MSG msg;
    if( GetMessage( &msg, 0, 0, 0 ))
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

void MessagePump::dispatchAll()
{
    _initReceiverQueue();

    MSG msg;
    while( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

}
}
