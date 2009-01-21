
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
     * The per-node event processing handler for agl windows.
     */
    class AGLEventHandler : public EventHandler
    {
    public:
        static AGLEventHandler* get();

        /** @sa EventHandler::registerWindow. */
        void registerWindow( AGLWindowIF* window );
        
        /** @sa EventHandler::deregisterWindow. */
        virtual void deregisterWindow( AGLWindowIF* window ) ;

    private:
        static AGLEventHandler _handler;

        /** Constructs a new agl event handler. */
        AGLEventHandler();

        /** Destructs the agl event handler. */
        virtual ~AGLEventHandler(){}
        
        static pascal OSStatus _handleEventUPP( EventHandlerCallRef nextHandler,
                                                EventRef event, void* userData);
        bool _handleEvent( EventRef event, AGLWindowIF* window );
        bool   _handleWindowEvent( EventRef event, AGLWindowIF* window );
        bool   _handleMouseEvent( EventRef event, AGLWindowIF* window );
        bool   _handleKeyEvent( EventRef event, AGLWindowIF* window );

        uint32_t _getButtonState();
        uint32_t _getButtonAction( EventRef event );
        uint32_t _getKey( EventRef event );

        uint32_t _lastDX;
        uint32_t _lastDY;
    };
}

#endif // EQ_AGLEVENTHANDLER_H

