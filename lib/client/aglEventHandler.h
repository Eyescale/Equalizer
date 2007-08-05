/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_AGLEVENTHANDLER_H
#define EQ_AGLEVENTHANDLER_H

#include <eq/client/eventHandler.h>

#include <eq/client/event.h>
#include <eq/net/command.h>
#include <eq/net/connectionSet.h>

namespace eq
{
    class X11Connection;

    /**
     * The per-node event processing handler for agl windows.
     */
    class AGLEventHandler : public EventHandler
    {
    public:
        static AGLEventHandler* get();

        /** @sa EventHandler::addWindow. */
        virtual void addWindow( Window* window );
        /** @sa EventHandler::removeWindow. */
        virtual void removeWindow( Window* window );

    private:
        static AGLEventHandler _handler;

        /** Constructs a new agl event handler. */
        AGLEventHandler();

        /** Destructs the agl event handler. */
        virtual ~AGLEventHandler(){}
        
        static pascal OSStatus _handleEventUPP( EventHandlerCallRef nextHandler,
                                                EventRef event, void* userData);
        bool _handleEvent( EventRef event, eq::Window* window );
        bool   _handleWindowEvent( EventRef event, eq::Window* window );
        bool   _handleMouseEvent( EventRef event, eq::Window* window );

        uint32_t _getButtonAction( EventRef event );

        uint32_t _lastDX;
        uint32_t _lastDY;
    };
}

#endif // EQ_AGLEVENTHANDLER_H

