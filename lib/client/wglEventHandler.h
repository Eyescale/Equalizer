
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WGLEVENTHANDLER_H
#define EQ_WGLEVENTHANDLER_H

#include <eq/client/eventHandler.h>

#include <eq/client/event.h>

namespace eq
{
    class WGLWindowIF;

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
        WGLEventHandler( WGLWindowIF* window );

        /** Destructs the wgl event handler. */
        virtual ~WGLEventHandler();

        static LRESULT CALLBACK wndProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                         LPARAM lParam );

        /** 
         * @return the function pointer of the previously installed window
         *         event handler function.
         */
        WNDPROC getPreviousWndProc() { return _prevWndProc; }

    private:
        WGLWindowIF* _window;
        HWND         _hWnd;
        WNDPROC      _prevWndProc;
        uint32_t     _buttonState;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        LRESULT CALLBACK _wndProc( HWND hWnd, UINT uMsg, WPARAM wParam, 
                                   LPARAM lParam );

        void      _syncButtonState( WPARAM wParam );
        uint32_t  _getKey( LPARAM lParam, WPARAM wParam );
    };
}

#endif // EQ_WGLEVENTHANDLER_H

