
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifndef EQ_GLXMESSAGEPUMP_H
#define EQ_GLXMESSAGEPUMP_H

#include <eq/client/messagePump.h>     // base class
#include <eq/client/glXEventHandler.h> // member [_wakeupSet]

namespace eq
{
    /**
     * Implements a message pump for the X11 window system.
     */
    class GLXMessagePump : public MessagePump
    {
    public:
        GLXMessagePump();

        /** Wake up dispatchOne(). */
        virtual void postWakeup();

        /** Get and dispatch all pending system events, non-blocking. */
        virtual void dispatchAll();

        /** Get and dispatch at least one pending system event, blocking. */
        virtual void dispatchOne();
        
        /** Clean up, no more dispatch from thread. */
        virtual void dispatchDone();
        
        virtual ~GLXMessagePump();

    private:
        GLXEventSetPtr _wakeupSet;
    };
}

#endif //EQ_GLXMESSAGEPUMP_H
