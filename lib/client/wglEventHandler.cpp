
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/base/base.h>      // first get windows.h
#include <pthread.h>           // then get pthreads
#include <eq/base/perThread.h> // then get perThread to have template code

#include "wglEventHandler.h"

#include "log.h"
#include "event.h"
#include "window.h"
#include "wglWindow.h"

#include <eq/base/debug.h>
#include <eq/base/executionListener.h>

#include <algorithm>
#include <windowsx.h>

using namespace eq::base;
using namespace std;
using namespace stde;

namespace eq
{

// Win32 defines to indentify special keys
#define SCANCODE_MASK     0xff0000
#define SCANCODE_SHIFT_L  0x2a0000
#define SCANCODE_SHIFT_R  0x360000
#define RIGHT_ALT_OR_CTRL 0x1000000

#ifndef MK_XBUTTON1
#  define MK_XBUTTON1  0x20
#endif
#ifndef MK_XBUTTON2
#  define MK_XBUTTON2  0x40
#endif
#ifndef GET_XBUTTON_WPARAM
#  define GET_XBUTTON_WPARAM(wParam) (HIWORD(wParam))
#endif
#ifndef GET_KEYSTATE_WPARAM
#  define GET_KEYSTATE_WPARAM(wParam) (LOWORD(wParam))
#endif


namespace
{
class HandlerMap
#ifdef _MSC_VER
    : public hash_map< HWND, WGLEventHandler* >
#else // Cygwin does not want to instantiate a hash with key=HWND
    : public hash_map< void*, WGLEventHandler* >
#endif
{
public:
    virtual ~HandlerMap() {}

    void notifyPerThreadDelete() 
        {
            if( !empty( ))
                EQWARN << size() 
                       << " WGL event handlers registered during thread exit"
                       << endl;
            delete this;
        }
};

static PerThread< HandlerMap > _handlers;



static void registerHandler( HWND hWnd, WGLEventHandler* handler )
{
    if( _handlers == 0 )
        _handlers = new HandlerMap;

    HandlerMap* map = _handlers.get();
    EQASSERT( map->find( hWnd ) == map->end( ));

    (*map)[hWnd] = handler;
}

static void deregisterHandler( HWND hWnd )
{
    HandlerMap* map = _handlers.get();
    EQASSERT( map )
    EQASSERT( map->find( hWnd ) != map->end( ));

    map->erase( hWnd );
}

static WGLEventHandler* getEventHandler( HWND hWnd )
{
    HandlerMap* map = _handlers.get();
    if( !map || map->find( hWnd ) == map->end( ))
        return 0;

    return (*map)[hWnd];
}
}

WGLEventHandler::WGLEventHandler( WGLWindowIF* window )
        : _window( window ),
          _buttonState( PTR_BUTTON_NONE )
{
    _hWnd = window->getWGLWindowHandle();

    if( !_hWnd )
    {
        EQWARN << "Window has no window handle" << endl;
        return;
    }

    registerHandler( _hWnd, this );
    _prevWndProc = (WNDPROC)SetWindowLongPtr( _hWnd, GWLP_WNDPROC, 
                                              (LONG_PTR)wndProc );
    if( _prevWndProc == wndProc ) // avoid recursion
        _prevWndProc = DefWindowProc;
}

WGLEventHandler::~WGLEventHandler()
{
    SetWindowLongPtr( _hWnd, GWLP_WNDPROC, (LONG_PTR)_prevWndProc );
    deregisterHandler( _hWnd );
}

LRESULT CALLBACK WGLEventHandler::wndProc( HWND hWnd, UINT uMsg, WPARAM wParam, 
                                           LPARAM lParam )
{
    WGLEventHandler* handler = getEventHandler( hWnd );
    if( !handler )
    {
        EQERROR << "Message arrived for unregistered window" << endl;
        return DefWindowProc( hWnd, uMsg, wParam, lParam );
    }

    return handler->_wndProc( hWnd, uMsg, wParam, lParam );
}

void WGLEventHandler::_syncButtonState( WPARAM wParam )
{
    uint32_t buttons = PTR_BUTTON_NONE;
    if( wParam & MK_LBUTTON )  buttons |= PTR_BUTTON1;
    if( wParam & MK_MBUTTON )  buttons |= PTR_BUTTON2;
    if( wParam & MK_RBUTTON )  buttons |= PTR_BUTTON3;
    if( wParam & MK_XBUTTON1 ) buttons |= PTR_BUTTON4;
    if( wParam & MK_XBUTTON2 ) buttons |= PTR_BUTTON5;

#ifndef NDEBUG
    if( _buttonState != buttons )

        EQWARN << "WM_MOUSEMOVE reports button state " << buttons
               << ", but internal state is " << _buttonState << endl;
#endif

    _buttonState = buttons;
}

namespace
{
void _getWindowSize( HWND hWnd, ResizeEvent& event )
{
    RECT rect;
    GetClientRect( hWnd, &rect );
    event.w = rect.right - rect.left;
    event.h = rect.bottom - rect.top; 

    // Get window coordinates, the rect data is relative
    // to window parent, but we report pvp relative to screen.
    POINT point;
    point.x = rect.left;
    point.y = rect.top;
    ClientToScreen( hWnd, &point );
    event.x = point.x;
    event.y = point.y;
}
}
LRESULT CALLBACK WGLEventHandler::_wndProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                            LPARAM lParam )
{
    WGLWindowEvent event;
    event.uMsg   = uMsg;
    event.wParam = wParam;
    event.lParam = lParam;

    Window* const window = _window->getWindow();

    LONG result = 0;
    switch( uMsg )
    {
        case WM_SHOWWINDOW:
            if( wParam == TRUE )
                event.type = Event::WINDOW_SHOW;
            else
                event.type = Event::WINDOW_HIDE;

            _getWindowSize( hWnd, event.resize );
            break;

        case WM_CREATE:
        case WM_SIZE:
        case WM_MOVE:
        case WM_WINDOWPOSCHANGED:
        {
            _getWindowSize( hWnd, event.resize );
            const bool hasArea = (event.resize.w >0 && event.resize.h > 0);
            const PixelViewport& pvp = window->getPixelViewport();

            // No show/hide events on Win32?: Emulate.
            if( !hasArea && pvp.hasArea( ))
                event.type = Event::WINDOW_HIDE;
            else if( hasArea && !pvp.hasArea( ))
                event.type = Event::WINDOW_SHOW;
            else
                event.type = Event::WINDOW_RESIZE;
            break;
        }

        case WM_CLOSE:
        case WM_DESTROY:
            event.type = Event::WINDOW_CLOSE;
            break;

        case WM_PAINT:
        {
            if( GetUpdateRect( hWnd, 0, false ) == 0 ) // No 'expose'
                return DefWindowProc( hWnd, uMsg, wParam, lParam );

            event.type = Event::WINDOW_EXPOSE;
            break;
        }

        case WM_MOUSEMOVE:
        {
            _syncButtonState( wParam );

            event.type = Event::POINTER_MOTION;
            event.pointerMotion.x = GET_X_LPARAM( lParam );
            event.pointerMotion.y = GET_Y_LPARAM( lParam );
            event.pointerMotion.buttons = _buttonState;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;
        }

        case WM_LBUTTONDOWN:
            _buttonState |= PTR_BUTTON1;
            event.type = Event::POINTER_BUTTON_PRESS;
            event.pointerButtonPress.x       = GET_X_LPARAM( lParam );
            event.pointerButtonPress.y       = GET_Y_LPARAM( lParam );
            event.pointerButtonPress.buttons = _buttonState;
            event.pointerButtonPress.button  = PTR_BUTTON1;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;

        case WM_MBUTTONDOWN:
            _buttonState |= PTR_BUTTON2;
            event.type = Event::POINTER_BUTTON_PRESS;
            event.pointerButtonPress.x       = GET_X_LPARAM( lParam );
            event.pointerButtonPress.y       = GET_Y_LPARAM( lParam );
            event.pointerButtonPress.buttons = _buttonState;
            event.pointerButtonPress.button  = PTR_BUTTON2;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;

        case WM_RBUTTONDOWN:
            _buttonState |= PTR_BUTTON3;
            event.type = Event::POINTER_BUTTON_PRESS;
            event.pointerButtonPress.x       = GET_X_LPARAM( lParam );
            event.pointerButtonPress.y       = GET_Y_LPARAM( lParam );
            event.pointerButtonPress.buttons = _buttonState;
            event.pointerButtonPress.button  = PTR_BUTTON3;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;

        case WM_XBUTTONDOWN:
            event.type = Event::POINTER_BUTTON_PRESS;
            event.pointerButtonPress.x       = GET_X_LPARAM( lParam );
            event.pointerButtonPress.y       = GET_Y_LPARAM( lParam );

            if( GET_XBUTTON_WPARAM( wParam ) & XBUTTON1 )
                event.pointerButtonRelease.button = PTR_BUTTON4;
            else
                event.pointerButtonRelease.button = PTR_BUTTON5;

            _buttonState |= event.pointerButtonPress.button;
            _syncButtonState( GET_KEYSTATE_WPARAM( wParam ));
            event.pointerButtonPress.buttons = _buttonState;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            result = TRUE;
            break;

        case WM_LBUTTONUP:
            _buttonState &= ~PTR_BUTTON1;
            event.type = Event::POINTER_BUTTON_RELEASE;
            event.pointerButtonRelease.x       = GET_X_LPARAM( lParam );
            event.pointerButtonRelease.y       = GET_Y_LPARAM( lParam );
            event.pointerButtonRelease.buttons = _buttonState;
            event.pointerButtonRelease.button  = PTR_BUTTON1;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;

        case WM_MBUTTONUP:
            _buttonState &= ~PTR_BUTTON2;
            event.type = Event::POINTER_BUTTON_RELEASE;
            event.pointerButtonRelease.x       = GET_X_LPARAM( lParam );
            event.pointerButtonRelease.y       = GET_Y_LPARAM( lParam );
            event.pointerButtonRelease.buttons = _buttonState;
            event.pointerButtonRelease.button  = PTR_BUTTON2;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;

        case WM_RBUTTONUP:
            _buttonState &= ~PTR_BUTTON3;
            event.type = Event::POINTER_BUTTON_RELEASE;
            event.pointerButtonRelease.x       = GET_X_LPARAM( lParam );
            event.pointerButtonRelease.y       = GET_Y_LPARAM( lParam );
            event.pointerButtonRelease.buttons = _buttonState;
            event.pointerButtonRelease.button  = PTR_BUTTON3;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;

        case WM_XBUTTONUP:
            event.type = Event::POINTER_BUTTON_RELEASE;
            event.pointerButtonRelease.x       = GET_X_LPARAM( lParam );
            event.pointerButtonRelease.y       = GET_Y_LPARAM( lParam );

            if( GET_XBUTTON_WPARAM( wParam ) & XBUTTON1 )
                event.pointerButtonRelease.button = PTR_BUTTON4;
            else
                event.pointerButtonRelease.button = PTR_BUTTON5;

            _buttonState &= ~event.pointerButtonRelease.button;
            _syncButtonState( GET_KEYSTATE_WPARAM( wParam ));
            event.pointerButtonRelease.buttons =_buttonState;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            result = TRUE;
            break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            event.type = Event::KEY_PRESS;
            event.keyPress.key = _getKey( lParam, wParam );
            break;

        case WM_SYSKEYUP:
        case WM_KEYUP:
            event.type = Event::KEY_RELEASE;
            event.keyRelease.key = _getKey( lParam, wParam );
            break;

        case WM_SYSCOMMAND:
            switch( wParam )
            {
                case SC_MONITORPOWER:
                case SC_SCREENSAVE:
                    if( lParam >= 0 ) // request off
                    {
                        event.type = Event::WINDOW_SCREENSAVER;
                        break;
                    }
                    // else no break; fall through
                default:
                    event.type = Event::UNKNOWN;
                    EQVERB << "Unhandled system command 0x" << hex << wParam 
                           << dec << endl;
                    break;
            }
            break;

        default:
            event.type = Event::UNKNOWN;
            EQVERB << "Unhandled message 0x" << hex << uMsg << dec << endl;
            break;
    }

    event.originator = window->getID();

    EQLOG( LOG_EVENTS ) << "received event: " << event << endl;

    if( _window->processEvent( event ))
        return result;

    return CallWindowProc( _prevWndProc, hWnd, uMsg, wParam, lParam );
}

uint32_t WGLEventHandler::_getKey( LPARAM lParam, WPARAM wParam )
{
    switch( wParam )
    {
        case VK_ESCAPE:    return KC_ESCAPE;    
        case VK_BACK:      return KC_BACKSPACE; 
        case VK_RETURN:    return KC_RETURN;    
        case VK_TAB:       return KC_TAB;       
        case VK_HOME:      return KC_HOME;       
        case VK_LEFT:      return KC_LEFT;       
        case VK_UP:        return KC_UP;         
        case VK_RIGHT:     return KC_RIGHT;      
        case VK_DOWN:      return KC_DOWN;       
        case VK_PRIOR:     return KC_PAGE_UP;    
        case VK_NEXT:      return KC_PAGE_DOWN;  
        case VK_END:       return KC_END;        
        case VK_F1:        return KC_F1;         
        case VK_F2:        return KC_F2;         
        case VK_F3:        return KC_F3;         
        case VK_F4:        return KC_F4;         
        case VK_F5:        return KC_F5;         
        case VK_F6:        return KC_F6;         
        case VK_F7:        return KC_F7;         
        case VK_F8:        return KC_F8;         
        case VK_F9:        return KC_F9;         
        case VK_F10:       return KC_F10;        
        case VK_F11:       return KC_F11;        
        case VK_F12:       return KC_F12;        
        case VK_F13:       return KC_F13;        
        case VK_F14:       return KC_F14;        
        case VK_F15:       return KC_F15;        
        case VK_F16:       return KC_F16;        
        case VK_F17:       return KC_F17;        
        case VK_F18:       return KC_F18;        
        case VK_F19:       return KC_F19;        
        case VK_F20:       return KC_F20;        
        case VK_F21:       return KC_F21;        
        case VK_F22:       return KC_F22;        
        case VK_F23:       return KC_F23;        
        case VK_F24:       return KC_F24;  
        case VK_SHIFT:
            switch( lParam & SCANCODE_MASK )
            {
                case SCANCODE_SHIFT_L: return KC_SHIFT_L;
                case SCANCODE_SHIFT_R: return KC_SHIFT_R;
            }
            break;

        case VK_CONTROL:
            if( lParam & RIGHT_ALT_OR_CTRL )
                return KC_CONTROL_R;
            return KC_CONTROL_L;

        case VK_MENU:
            if( lParam & RIGHT_ALT_OR_CTRL )
                return KC_ALT_R;
            return KC_ALT_L;

        default: 
            // 'Useful' Latin1 characters
            if( wParam >= ' ' && wParam <= '~' )
                return wParam;
            break;
    }
    EQWARN << "Unrecognized virtual key code " << wParam << endl;
    return KC_VOID;
}
}
