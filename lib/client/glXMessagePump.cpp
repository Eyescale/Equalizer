
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

void GLXMessagePump::dispatchDone()
{
    if( !_wakeupSet )
        return;

    _wakeupSet = 0;
    GLXEventHandler::clearEventSet();
}

}
