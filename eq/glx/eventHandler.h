
/* Copyright (c) 2006-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2014, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_GLX_EVENTHANDLER_H
#define EQ_GLX_EVENTHANDLER_H

#include <eq/glx/types.h>
#include <eq/eventHandler.h> // base class
#include <eq/types.h>

#include <lunchbox/thread.h> // thread-safety macro

namespace eq
{
namespace glx
{
    /** The event handler for glX/X11 windows. */
    class EventHandler : public eq::EventHandler
    {
    public:
        /** Construct a new glX event handler. @version 1.0 */
        EventHandler( WindowIF* window );

        /** Destruct the glX event handler. @version 1.0 */
        virtual ~EventHandler();

        /**
         * Dispatch all pending events on the current thread.
         *
         * If no event handlers have been constructed by the calling thread,
         * this function does nothing. This function does not block on events.
         * @version 1.0
         */
        static void dispatch();

    private:
        WindowIF* const _window;

        bool _magellanUsed; //!< Window registered with spnav

        void _dispatch();
        void _processEvent( WindowEvent& event );
        uint32_t _getButtonState( XEvent& event );
        uint32_t _getButtonAction( XEvent& event );
        uint32_t _getKey( XEvent& event );

        LB_TS_VAR( _thread );
    };
}
}
#endif // EQ_GLX_EVENTHANDLER_H
