
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_GLXMESSAGEPUMP_H
#define EQ_GLXMESSAGEPUMP_H

#include <eq/messagePump.h> // base class
#include <eq/os.h>          // X11

#include <co/connectionSet.h>  // member

namespace eq
{
    /** A message pump receiving and dispatching X11 events. */
    class GLXMessagePump : public MessagePump
    {
    public:
        /** Construct a new X11 message pump. @version 1.0 */
        GLXMessagePump();

        /** Destruct this message pump. @version 1.0 */
        virtual ~GLXMessagePump();

        virtual void postWakeup();
        virtual void dispatchAll();
        virtual void dispatchOne();
        
        /**
         * Register a new Display connection for event dispatch.
         *
         * The registrations are referenced, that is, multiple registrations of
         * the same display cause the Display to be added once to the event
         * set, but require the same amount of deregistrations to stop event
         * dispatch on the connection. Not threadsafe.
         *
         * @sa GLXEventHandler
         * @version 1.0
         */
        void register_( Display* display );

        /** Deregister a Display connection from event dispatch. @version 1.0 */
        void deregister( Display* display );

    private:
        co::ConnectionSet _connections; //!< Registered Display connections
        stde::hash_map< void*, size_t > _referenced; //!< # of registrations
    };
}

#endif //EQ_GLXMESSAGEPUMP_H
