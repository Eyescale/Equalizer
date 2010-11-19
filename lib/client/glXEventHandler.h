
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
#include <eq/client/os.h>           // XEvent type
#include <eq/client/types.h>        // basic typedefs

#include <eq/base/thread.h> // thread-safety macro

namespace eq
{
    /** The event handler for glX/X11 windows. */
    class GLXEventHandler : public EventHandler
    {
    public:
        /** Dispatch all pending events on the current thread, non-blocking. */
        static void dispatch();

        /** Construct a new glx event handler. */
        GLXEventHandler( GLXWindowIF* window );

        /** Destructs the glX event handler. */
        virtual ~GLXEventHandler();

    private:
        /** The corresponding glX pipe. */
        GLXWindowIF* const _window;

        void _dispatch();
        void _processEvent( GLXWindowEvent& event );
        uint32_t _getButtonState( XEvent& event );
        uint32_t _getButtonAction( XEvent& event );
        uint32_t _getKey( XEvent& event );

        EQ_TS_VAR( _thread );
    };
}

#endif // EQ_GLXEVENTHANDLER_H

