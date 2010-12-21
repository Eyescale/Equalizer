
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "wglMessagePump.h"

#include <co/base/debug.h>
#include <co/base/log.h>

using namespace std;

namespace eq
{
WGLMessagePump::WGLMessagePump()
        : _win32ThreadID( 0 )
{
}

void WGLMessagePump::postWakeup()
{
    if( !_win32ThreadID )
    {
        EQWARN << "Receiver thread not waiting?" << endl;
        return;
    }

    PostThreadMessage( _win32ThreadID, WM_APP, 0, 0 ); // Wake up pop()
}

void WGLMessagePump::_initReceiverQueue()
{
    if( !_win32ThreadID )
    {
        MSG msg;
        PeekMessage( &msg, 0, WM_USER, WM_USER, PM_NOREMOVE );
        _win32ThreadID = GetCurrentThreadId();
    }
    EQASSERTINFO( _win32ThreadID == GetCurrentThreadId(),
                  "WGLMessagePump::pop() called from two different threads" );
}

void WGLMessagePump::dispatchOne()
{
    _initReceiverQueue();

    MSG msg;
    if( GetMessage( &msg, 0, 0, 0 ))
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

void WGLMessagePump::dispatchAll()
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
