
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_WGLEVENTHANDLER_H
#define EQ_WGLEVENTHANDLER_H

#include <eq/client/eventHandler.h> // base class

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
    class WGLEventHandler : public EventHandler
    {
    public:
        /** Constructs a new WGL event handler. */
        EQ_EXPORT WGLEventHandler( WGLWindowIF* window );

        /** Destructs the WGL event handler. */
        EQ_EXPORT virtual ~WGLEventHandler();

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

        static LRESULT CALLBACK wndProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                         LPARAM lParam );
        LRESULT CALLBACK _wndProc( HWND hWnd, UINT uMsg, WPARAM wParam, 
                                   LPARAM lParam );

        void      _syncButtonState( WPARAM wParam );
        uint32_t  _getKey( LPARAM lParam, WPARAM wParam );
    };
}

#endif // EQ_WGLEVENTHANDLER_H

