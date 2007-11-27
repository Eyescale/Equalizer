/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WGLEVENTHANDLER_H
#define EQ_WGLEVENTHANDLER_H

#include <eq/client/eventHandler.h>

#include <eq/client/event.h>
#include <eq/client/windowEvent.h>

namespace eq
{
    /**
     * The event processing for WGL.
     *
     * The WGL implementation does not use a thread, since messages are handled
     * by a 'wndproc' callback in the thread which created the window. Each
     * window has its own WGLEventHandler
     */
    class EQ_EXPORT WGLEventHandler : public EventHandler
    {
    public:
        /** Constructs a new wgl event handler. */
        WGLEventHandler( Window* window );

        /** @sa EventHandler::deregisterPipe() */
        virtual void deregisterPipe( Pipe* pipe ){ /*NOP*/ }
        
        /** @sa EventHandler::deregisterWindow() */
        virtual void deregisterWindow( Window* window ){ delete this; }

        static LRESULT CALLBACK wndProc( HWND hWnd, UINT uMsg, WPARAM wParam, 
                                         LPARAM lParam );

      protected:
        /** Destructs the wgl event handler. */
        virtual ~WGLEventHandler();
        
    private:
		Window*  _window;
        HWND     _hWnd;
        WNDPROC  _prevWndProc;
        uint32_t _buttonState;

        LRESULT CALLBACK _wndProc( HWND hWnd, UINT uMsg, WPARAM wParam, 
                                   LPARAM lParam );

        void      _syncButtonState( WPARAM wParam );
        uint32_t  _getKey( LPARAM lParam, WPARAM wParam );
    };
}

#endif // EQ_WGLEVENTHANDLER_H

