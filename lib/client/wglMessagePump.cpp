/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "wglMessagePump.h"

#include <eq/base/debug.h>
#include <eq/base/log.h>

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
