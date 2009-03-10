
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_AGLEVENTHANDLER_H
#define EQ_AGLEVENTHANDLER_H

#include <eq/client/eventHandler.h> // base class

namespace eq
{
    class AGLWindowIF;
    class X11Connection;

    /**
     * The event handler for agl windows.
     */
    class AGLEventHandler : public EventHandler
    {
    public:
        /** Construct a new AGL event handler for the given AGL window. */
        AGLEventHandler( AGLWindowIF* window );
        
        /** @sa EventHandler::deregisterWindow. */
        virtual ~AGLEventHandler();

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

