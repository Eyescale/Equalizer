
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_GLXEVENTHANDLER_H
#define EQ_GLXEVENTHANDLER_H

#include <eq/client/eventHandler.h> // base class
#include <eq/client/types.h>        // basic typedefs
#include <eq/client/windowSystem.h> // XEvent type

#include <eq/net/connectionSet.h>

namespace eq
{
    class GLXPipe;
    class Window;
    class GLXWindowEvent;

    /**
     * The event handler for glX.
     */
    class GLXEventHandler : public EventHandler
    {
    public:
        class EventSet : public net::ConnectionSet, public base::Referenced
        {
        public:
            void notifyPerThreadDelete() { unref(); }

        protected:
            virtual ~EventSet(){}
        };

        /** Dispatch at least one event for the current thread, blocking. */
        static void dispatchOne();

        /** Dispatch all pending events on the current thread, non-blocking. */
        static void dispatchAll();

        /** Get the event set of the current thread. */
        static base::RefPtr< EventSet > getEventSet();

        /** Clear the event set of the current thread. */
        static void clearEventSet();

        /** Constructs a new glx event handler. */
        GLXEventHandler( GLXPipe* pipe );

        /** Destructs the glX event handler. */
        virtual ~GLXEventHandler();

    private:
        /** The corresponding glX pipe. */
        GLXPipe* const _pipe;

        static void _handleEvents( X11ConnectionPtr connection );

        void _processEvent( GLXWindowEvent& event, Pipe* pipe );
        uint32_t  _getButtonState( XEvent& event );
        uint32_t  _getButtonAction( XEvent& event );
        uint32_t  _getKey( XEvent& event );
    };

    /** @cond IGNORE */
    typedef base::RefPtr< GLXEventHandler::EventSet > GLXEventSetPtr; 
    /** @endcond */
}

#endif // EQ_GLXEVENTHANDLER_H

