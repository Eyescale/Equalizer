/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "glXMessagePump.h"

#include "glXEventHandler.h"

#include <eq/base/debug.h>
#include <eq/base/log.h>

using namespace std;

namespace eq
{
GLXMessagePump::GLXMessagePump()
        :  _wakeupSet( 0 )
{
}

GLXMessagePump::~GLXMessagePump()
{
    _wakeupSet = 0;
}

void GLXMessagePump::postWakeup()
{
    if( !_wakeupSet )
    {
        EQWARN << "Receiver thread not waiting?" << endl;
        return;
    }

    _wakeupSet->interrupt();
}

void GLXMessagePump::dispatchOne()
{
    if( !_wakeupSet )
        _wakeupSet = GLXEventHandler::getEventSet();

    GLXEventHandler::dispatchOne();
}

void GLXMessagePump::dispatchAll()
{
    if( !_wakeupSet )
        _wakeupSet = GLXEventHandler::getEventSet();

    GLXEventHandler::dispatchAll();
}
}
