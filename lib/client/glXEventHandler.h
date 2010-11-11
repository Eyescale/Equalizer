
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <eq/client/os.h>           // XEvent type

#include <eq/net/connectionSet.h>

namespace eq
{
    class GLXPipe;
    class GLXWindowEvent;
    class GLXWindowIF;
    class Window;

    /** The event handler for glX/X11 windows. */
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

        /** Construct a new glx event handler. */
        GLXEventHandler( GLXPipe* pipe );

        /** Destructs the glX event handler. */
        virtual ~GLXEventHandler();

        /** Register a window for event handling. */
        void registerWindow( GLXWindowIF* window );

        /** Deregister a window from event handling. */
        void deregisterWindow( GLXWindowIF* window );

    private:
        /** The corresponding glX pipe. */
        GLXPipe* const _pipe;

        typedef stde::hash_map< XID, GLXWindowIF* > WindowMap;

        /** Registered windows. */
        WindowMap _windows;

        static void _handleEvents( X11ConnectionPtr connection );

        /** @return true if something was handled, false on timeout */
        static bool _dispatch( const int timeout );

        void _processEvent( GLXWindowEvent& event );
        uint32_t  _getButtonState( XEvent& event );
        uint32_t  _getButtonAction( XEvent& event );
        uint32_t  _getKey( XEvent& event );

        EQ_TS_VAR( _thread );
    };

    /** @cond IGNORE */
    typedef base::RefPtr< GLXEventHandler::EventSet > GLXEventSetPtr; 
    /** @endcond */
}

#endif // EQ_GLXEVENTHANDLER_H

