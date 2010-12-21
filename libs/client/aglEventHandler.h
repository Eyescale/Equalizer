
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

#ifndef EQ_AGLEVENTHANDLER_H
#define EQ_AGLEVENTHANDLER_H

#include <eq/eventHandler.h> // base class
#include <eq/os.h>           // AGL-specific types

namespace eq
{
    class AGLWindowIF;
    class Node;

    /** 
     * The event handler for AGL windows.
     *
     * Any implementation of the AGLWindowIF can instantiate this event handler,
     * which registers for Carbon events, translates each received event to an
     * AGLWindowEvent and dispatches it to AGLWindowIF::processEvent.
     */
    class AGLEventHandler : public EventHandler
    {
    public:
        /**
         * Construct a new AGL event handler for the given AGL window.
         * @version 1.0
         */
        AGLEventHandler( AGLWindowIF* window );
        
        /** Destruct the AGL event handler. @version 1.0 */
        virtual ~AGLEventHandler();

        /**
         * Initialize space mouse event handling for this process.
         *
         * Received space mouse events are directly send to the application.
         * @sa Node::configInit, Config::sendEvent
         * @version 1.0
         */
        static void initMagellan( Node* node );

        /**
         * De-initialize space mouse event handling for this process.
         * @sa Node::configInit
         * @version 1.0
         */
        static void exitMagellan( Node* node );

    private:
        AGLWindowIF* const _window;

        EventHandlerRef _eventHandler;
        EventHandlerRef _eventDispatcher;

        static pascal OSStatus _dispatchEventUPP( 
            EventHandlerCallRef nextHandler, EventRef event, void* userData );

        static pascal OSStatus _handleEventUPP( EventHandlerCallRef nextHandler,
                                                EventRef event, void* userData);
        bool _handleEvent( EventRef event );
        bool   _handleWindowEvent( EventRef event );
        bool   _handleMouseEvent( EventRef event );
        bool   _handleKeyEvent( EventRef event );

        uint32_t _getButtonState();
        uint32_t _getButtonAction( EventRef event );
        uint32_t _getKey( EventRef event );

        uint32_t _lastDX;
        uint32_t _lastDY;
    };
}

#endif // EQ_AGLEVENTHANDLER_H

