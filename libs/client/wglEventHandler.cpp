
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2011, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include <co/base/os.h>      // first get windows.h
#include <pthread.h>           // then get pthreads
#include <co/base/perThread.h> // then get perThread to have template code

#include "wglEventHandler.h"
#include "config.h"
#include "configEvent.h"
#include "event.h"
#include "log.h"
#include "node.h"
#include "wglWindow.h"
#include "window.h"

#include <co/base/debug.h>
#include <co/base/executionListener.h>

#include <algorithm>
#include <windowsx.h>

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
#ifdef _MSC_VER
    typedef stde::hash_map< HWND, WGLEventHandler* > HandlerMap;
#else // Cygwin does not want to instantiate a hash with key=HWND
    typedef stde::hash_map< void*, WGLEventHandler* > HandlerMap;
#endif

static co::base::PerThread< HandlerMap > _handlers;

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
        EQWARN << "Window has no window handle" << std::endl;
        return;
    }

    registerHandler( _hWnd, this );

#pragma warning(push)
#pragma warning(disable: 4312)
    _prevWndProc = (WNDPROC)SetWindowLongPtr( _hWnd, GWLP_WNDPROC, 
                                              (LONG_PTR)wndProc );
#pragma warning(pop) 

    if( _prevWndProc == wndProc ) // avoid recursion
        _prevWndProc = DefWindowProc;

    UINT scrollLines;
    SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &scrollLines, 0 );
    _wheelDeltaPerLine = WHEEL_DELTA / scrollLines;
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
        EQERROR << "Message arrived for unregistered window" << std::endl;
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
               << ", but internal state is " << _buttonState << std::endl;
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

            event.type = Event::WINDOW_POINTER_MOTION;
            event.pointerMotion.x = GET_X_LPARAM( lParam );
            event.pointerMotion.y = GET_Y_LPARAM( lParam );
            event.pointerMotion.buttons = _buttonState;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;
        }

        case WM_LBUTTONDOWN:
            _buttonState |= PTR_BUTTON1;
            event.type = Event::WINDOW_POINTER_BUTTON_PRESS;
            event.pointerButtonPress.x       = GET_X_LPARAM( lParam );
            event.pointerButtonPress.y       = GET_Y_LPARAM( lParam );
            event.pointerButtonPress.buttons = _buttonState;
            event.pointerButtonPress.button  = PTR_BUTTON1;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;

        case WM_MBUTTONDOWN:
            _buttonState |= PTR_BUTTON2;
            event.type = Event::WINDOW_POINTER_BUTTON_PRESS;
            event.pointerButtonPress.x       = GET_X_LPARAM( lParam );
            event.pointerButtonPress.y       = GET_Y_LPARAM( lParam );
            event.pointerButtonPress.buttons = _buttonState;
            event.pointerButtonPress.button  = PTR_BUTTON2;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;

        case WM_RBUTTONDOWN:
            _buttonState |= PTR_BUTTON3;
            event.type = Event::WINDOW_POINTER_BUTTON_PRESS;
            event.pointerButtonPress.x       = GET_X_LPARAM( lParam );
            event.pointerButtonPress.y       = GET_Y_LPARAM( lParam );
            event.pointerButtonPress.buttons = _buttonState;
            event.pointerButtonPress.button  = PTR_BUTTON3;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;

        case WM_XBUTTONDOWN:
            event.type = Event::WINDOW_POINTER_BUTTON_PRESS;
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
            event.type = Event::WINDOW_POINTER_BUTTON_RELEASE;
            event.pointerButtonRelease.x       = GET_X_LPARAM( lParam );
            event.pointerButtonRelease.y       = GET_Y_LPARAM( lParam );
            event.pointerButtonRelease.buttons = _buttonState;
            event.pointerButtonRelease.button  = PTR_BUTTON1;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;

        case WM_MBUTTONUP:
            _buttonState &= ~PTR_BUTTON2;
            event.type = Event::WINDOW_POINTER_BUTTON_RELEASE;
            event.pointerButtonRelease.x       = GET_X_LPARAM( lParam );
            event.pointerButtonRelease.y       = GET_Y_LPARAM( lParam );
            event.pointerButtonRelease.buttons = _buttonState;
            event.pointerButtonRelease.button  = PTR_BUTTON2;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;

        case WM_RBUTTONUP:
            _buttonState &= ~PTR_BUTTON3;
            event.type = Event::WINDOW_POINTER_BUTTON_RELEASE;
            event.pointerButtonRelease.x       = GET_X_LPARAM( lParam );
            event.pointerButtonRelease.y       = GET_Y_LPARAM( lParam );
            event.pointerButtonRelease.buttons = _buttonState;
            event.pointerButtonRelease.button  = PTR_BUTTON3;

            _computePointerDelta( window, event );
            _getRenderContext( window, event );
            break;

        case WM_XBUTTONUP:
            event.type = Event::WINDOW_POINTER_BUTTON_RELEASE;
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

        case WM_MOUSEWHEEL:
            event.type = Event::WINDOW_POINTER_WHEEL;
            event.pointerWheel.x     = GET_X_LPARAM( lParam );
            event.pointerWheel.y     = GET_Y_LPARAM( lParam );
            event.pointerWheel.buttons = _buttonState;
            event.pointerWheel.xAxis = _getWheelDelta( wParam );
            break;

#ifdef WM_MOUSEHWHEEL // only available on vista or later
        case WM_MOUSEHWHEEL:
            event.type = Event::WINDOW_POINTER_WHEEL;
            event.pointerWheel.x     = GET_X_LPARAM( lParam );
            event.pointerWheel.y     = GET_Y_LPARAM( lParam );
            event.pointerWheel.buttons = _buttonState;
            event.pointerWheel.yAxis = _getWheelDelta( wParam );
            break;
#endif

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
                    EQVERB << "Unhandled system command 0x" << std::hex
                           << wParam  << std::dec << std::endl;
                    break;
            }
            break;
#ifdef EQ_USE_MAGELLAN
        case WM_INPUT:
            _magellanEventHandler( lParam );
            break;
#endif
        default:
            event.type = Event::UNKNOWN;
            EQVERB << "Unhandled message 0x" << std::hex << uMsg << std::dec
                   << std::endl;
            break;
    }
    
    EQASSERT( window->getID() != co::base::UUID::ZERO );
    event.originator = window->getID();

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
            if( !(GetKeyState(VK_LSHIFT) & 0x1000 ) &&
                wParam >= 'A' && wParam <= 'Z' )
            {
                return wParam + 32;
            }
            if( wParam >= ' ' && wParam <= '~' )
                return wParam;
            break;
    }
    EQWARN << "Unrecognized virtual key code " << wParam << std::endl;
    return KC_VOID;
}

int32_t WGLEventHandler::_getWheelDelta( WPARAM wParam ) const
{
    const int32_t rawDelta = 
        static_cast< int32_t >( GET_WHEEL_DELTA_WPARAM( wParam ));
    return rawDelta / _wheelDeltaPerLine;
}


#ifdef EQ_USE_MAGELLAN

namespace
{
    static Node* _magellanNode = 0;

    static bool _magellanGotTranslation = false, _magellanGotRotation = false;
    static int _magellanDOFs[6] = {0};

    // ----------------  RawInput ------------------
    PRAWINPUTDEVICELIST _pRawInputDeviceList;
    PRAWINPUTDEVICE     _rawInputDevices;
    int                 _nRawInputDevices;
}
#endif

void WGLEventHandler::_magellanEventHandler( LPARAM lParam )
{
#ifdef EQ_USE_MAGELLAN
    RAWINPUTHEADER header;
    const UINT size_rawinputheader = sizeof( RAWINPUTHEADER );
    UINT size = sizeof(header);

    if( GetRawInputData( (HRAWINPUT)lParam, RID_HEADER, &header, &size,
                         size_rawinputheader ) == -1 )
    {
        EQINFO << "Error from GetRawInputData( RID_HEADER )" << std::endl;
        return;
    }

    // Ask Windows for the size of the event, because it is different on 32-bit
    // vs. 64-bit versions of the OS
    if( GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, 0, &size, 
                         size_rawinputheader ) == -1)
    {
        EQINFO << "Error from GetRawInputData(RID_INPUT)" << std::endl;
        return;
    }

    // Set aside enough memory for the full event
    LPRAWINPUT evt = (LPRAWINPUT)alloca( size );

    if( GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, evt, &size, 
                         size_rawinputheader ) == -1)
    {
        EQINFO << "Error from GetRawInputData(RID_INPUT)" << std::endl;
        return;
    }
    // else

    if (evt->header.dwType == RIM_TYPEHID)
    {           
        LPRAWHID pRawHid = &evt->data.hid;

        // translation or rotation packet?  They come in two different packets.
        if( pRawHid->bRawData[0] == 1) // Translation vector
        {
            _magellanDOFs[0] = (pRawHid->bRawData[1] & 0x000000ff) |
                ((signed short)(pRawHid->bRawData[2]<<8) & 0xffffff00); 
            _magellanDOFs[1] = (pRawHid->bRawData[3] & 0x000000ff) | 
                ((signed short)(pRawHid->bRawData[4]<<8) & 0xffffff00); 
            _magellanDOFs[2] = (pRawHid->bRawData[5] & 0x000000ff) |
                ((signed short)(pRawHid->bRawData[6]<<8) & 0xffffff00);
            _magellanGotTranslation = true;
        }
        else if (pRawHid->bRawData[0] == 2) // Rotation vector
        {
            _magellanDOFs[3] = (pRawHid->bRawData[1] & 0x000000ff) |
                ((signed short)(pRawHid->bRawData[2]<<8) & 0xffffff00); 
            _magellanDOFs[4] = (pRawHid->bRawData[3] & 0x000000ff) |
                ((signed short)(pRawHid->bRawData[4]<<8) & 0xffffff00); 
            _magellanDOFs[5] = (pRawHid->bRawData[5] & 0x000000ff) |
                ((signed short)(pRawHid->bRawData[6]<<8) & 0xffffff00);
            _magellanGotRotation = true;
        }
        else if (pRawHid->bRawData[0] == 3) // Buttons
        {
            ConfigEvent event;
            EQASSERT( _magellanNode->getID() != co::base::UUID::ZERO );
            event.data.originator = _magellanNode->getID();
            event.data.type = Event::MAGELLAN_BUTTON;
            event.data.magellan.button = 0;
            event.data.magellan.buttons = 0;
            // TODO: find fired button(s) since last buttons event
            
            if( pRawHid->bRawData[1] )
                event.data.magellan.buttons = PTR_BUTTON1;
            if( pRawHid->bRawData[2] )
                event.data.magellan.buttons |= PTR_BUTTON2;
            if( pRawHid->bRawData[3] )
                event.data.magellan.buttons |= PTR_BUTTON3;
            _magellanNode->getConfig()->sendEvent( event );
        }
        else
        {
            EQASSERTINFO( 0, "Unimplemented space mouse command " <<
                          pRawHid->bRawData[0] );
        }

        if (_magellanGotTranslation && _magellanGotRotation)
        {
            ConfigEvent event;
            EQASSERT( _magellanNode->getID() != co::base::UUID::ZERO );
            event.data.originator = _magellanNode->getID();
            event.data.type = Event::MAGELLAN_AXIS;
            event.data.magellan.xAxis = _magellanDOFs[0];
            event.data.magellan.yAxis = _magellanDOFs[1];
            event.data.magellan.zAxis = _magellanDOFs[2];
            event.data.magellan.xRotation = _magellanDOFs[3];
            event.data.magellan.yRotation = _magellanDOFs[4];
            event.data.magellan.zRotation = _magellanDOFs[5];

            _magellanGotRotation = false;
            _magellanGotTranslation = false;
            _magellanNode->getConfig()->sendEvent( event );
        }
    }
#endif
}

bool WGLEventHandler::initMagellan(Node* node)
{
#ifdef EQ_USE_MAGELLAN
    _magellanGotRotation = false;
    _magellanGotTranslation = false;
    
    // Find the Raw Devices
    UINT nDevices;

    // Get Number of devices attached
    if( GetRawInputDeviceList( 0, &nDevices, sizeof( RAWINPUTDEVICELIST )) != 0)
    { 
        EQINFO << "No RawInput devices attached" << std::endl;
        return false;
    }
    // Create list large enough to hold all RAWINPUTDEVICE structs
    _pRawInputDeviceList = (PRAWINPUTDEVICELIST)malloc(
        sizeof( RAWINPUTDEVICELIST ) * nDevices );
    if( !_pRawInputDeviceList )
    {
        EQINFO << "Error mallocing RAWINPUTDEVICELIST" << std::endl;
        return false;
    }
    // Now get the data on the attached devices
    if( GetRawInputDeviceList( _pRawInputDeviceList, &nDevices, 
                               sizeof( RAWINPUTDEVICELIST )) == -1) 
    {
        EQINFO << "Error from GetRawInputDeviceList" << std::endl;
        return false;
    }

    _rawInputDevices = (PRAWINPUTDEVICE)alloca( nDevices *
                                                sizeof( RAWINPUTDEVICE ));
    _nRawInputDevices = 0;

    // Look through device list for RIM_TYPEHID devices with UsagePage == 1,
    // Usage == 8
    for( UINT i=0; i<nDevices; ++i )
    {
        if( _pRawInputDeviceList[i].dwType == RIM_TYPEHID )
        {
            UINT nchars = 300;
            TCHAR deviceName[300];
            if( GetRawInputDeviceInfo( _pRawInputDeviceList[i].hDevice,
                                       RIDI_DEVICENAME, deviceName, 
                                       &nchars) >= 0)
            {
                EQINFO << "Device [" << i << "]: handle=" 
                       << _pRawInputDeviceList[i].hDevice << " name = "
                       << deviceName << std::endl;
            }

            RID_DEVICE_INFO dinfo;
            UINT sizeofdinfo = sizeof(dinfo);
            dinfo.cbSize = sizeofdinfo;
            if( GetRawInputDeviceInfo( _pRawInputDeviceList[i].hDevice,
                                       RIDI_DEVICEINFO, &dinfo,
                                       &sizeofdinfo ) >= 0)
            {
                if (dinfo.dwType == RIM_TYPEHID)
                {
                    RID_DEVICE_INFO_HID *phidInfo = &dinfo.hid;
                    EQINFO << "VID = " << phidInfo->dwVendorId << std::endl
                           << "PID = " << phidInfo->dwProductId << std::endl
                           << "Version = " << phidInfo->dwVersionNumber 
                           << std::endl
                           << "UsagePage = " << phidInfo->usUsagePage
                           << std::endl
                           << "Usage = " << phidInfo->usUsage << std::endl;

                    // Add this one to the list of interesting devices?
                    // Actually only have to do this once to get input from all
                    // usage 1, usagePage 8 devices This just keeps out the
                    // other usages.  You might want to put up a list for users
                    // to select amongst the different devices.  In particular,
                    // to assign separate functionality to the different
                    // devices.
                    if (phidInfo->usUsagePage == 1 && phidInfo->usUsage == 8)
                    {
                        _rawInputDevices[_nRawInputDevices].usUsagePage = 
                            phidInfo->usUsagePage;
                        _rawInputDevices[_nRawInputDevices].usUsage     = 
                            phidInfo->usUsage;
                        _rawInputDevices[_nRawInputDevices].dwFlags     = 0;
                        _rawInputDevices[_nRawInputDevices].hwndTarget  = 0;
                        ++_nRawInputDevices;
                    }
                }
            }
        }
    }

    // Register for input from the devices in the list
    if( RegisterRawInputDevices( _rawInputDevices, _nRawInputDevices,
                                 sizeof(RAWINPUTDEVICE) ) == FALSE )
    {
        EQINFO << "Error calling RegisterRawInputDevices" << std::endl;
        return false;
    }

    EQINFO << "Found and connected." << std::endl;
     _magellanNode = node;
#endif
    return true;
}

void WGLEventHandler::exitMagellan(eq::Node *node)
{
#ifdef EQ_USE_MAGELLAN
    if( _magellanNode == node )
    {
        free(_pRawInputDeviceList);
        _magellanNode = 0;
    }
#endif
}

}
