
/* Copyright (c) 2007-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#include <lunchbox/os.h>      // first get windows.h
#include <lunchbox/perThread.h> // then get perThread to have template code

#include "eventHandler.h"

#include "window.h"

#include "../config.h"
#include "../log.h"
#include "../node.h"
#include "../window.h"

#include <eq/fabric/axisEvent.h>
#include <eq/fabric/buttonEvent.h>
#include <eq/fabric/keyEvent.h>
#include <eq/fabric/sizeEvent.h>

#include <lunchbox/debug.h>

#include <algorithm>
#include <windowsx.h>

namespace eq
{
namespace wgl
{

// Win32 defines to identify special keys
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
    typedef stde::hash_map< HWND, EventHandler* > HandlerMap;
#else // Cygwin does not want to instantiate a hash with key=HWND
    typedef stde::hash_map< void*, EventHandler* > HandlerMap;
#endif

lunchbox::PerThread< HandlerMap > _handlers;

void _registerHandler( HWND hWnd, EventHandler* handler )
{
    if( _handlers == 0 )
        _handlers = new HandlerMap;

    HandlerMap* map = _handlers.get();
    LBASSERT( map->find( hWnd ) == map->end( ));

    (*map)[hWnd] = handler;
}

void _deregisterHandler( HWND hWnd )
{
    HandlerMap* map = _handlers.get();
    LBASSERT( map )
    LBASSERT( map->find( hWnd ) != map->end( ));

    map->erase( hWnd );
}

EventHandler* _getEventHandler( HWND hWnd )
{
    HandlerMap* map = _handlers.get();
    if( !map || map->find( hWnd ) == map->end( ))
        return 0;

    return (*map)[hWnd];
}

uint32_t _getKey( LPARAM lParam, WPARAM wParam )
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
    LBWARN << "Unrecognized virtual key code " << wParam << std::endl;
    return KC_VOID;
}

KeyModifier _getKeyModifiers()
{
    KeyModifier result = KeyModifier::none;
    if( GetKeyState( VK_MENU ) & 0x8000 )
        result |= KeyModifier::alt;
    if( GetKeyState( VK_CONTROL ) & 0x8000 )
        result |= KeyModifier::control;
    if( GetKeyState( VK_SHIFT ) & 0x8000 )
        result |= KeyModifier::shift;
    return result;
}

}

EventHandler::EventHandler( WindowIF* window )
        : _window( window ),
          _buttonState( PTR_BUTTON_NONE )
{
    _hWnd = window->getWGLWindowHandle();

    if( !_hWnd )
    {
        LBWARN << "Window has no window handle" << std::endl;
        return;
    }

    _registerHandler( _hWnd, this );

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

EventHandler::~EventHandler()
{
    SetWindowLongPtr( _hWnd, GWLP_WNDPROC, (LONG_PTR)_prevWndProc );
    _deregisterHandler( _hWnd );
}

LRESULT CALLBACK EventHandler::wndProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                        LPARAM lParam )
{
    EventHandler* handler = _getEventHandler( hWnd );
    if( !handler )
    {
        LBERROR << "Message arrived for unregistered window" << std::endl;
        return DefWindowProc( hWnd, uMsg, wParam, lParam );
    }

    return handler->_wndProc( hWnd, uMsg, wParam, lParam );
}

void EventHandler::_syncButtonState( WPARAM wParam )
{
    uint32_t buttons = PTR_BUTTON_NONE;
    if( wParam & MK_LBUTTON )  buttons |= PTR_BUTTON1;
    if( wParam & MK_RBUTTON )  buttons |= PTR_BUTTON2;
    if( wParam & MK_MBUTTON )  buttons |= PTR_BUTTON3;
    if( wParam & MK_XBUTTON1 ) buttons |= PTR_BUTTON4;
    if( wParam & MK_XBUTTON2 ) buttons |= PTR_BUTTON5;

#ifndef NDEBUG
    if( _buttonState != buttons )

        LBWARN << "WM_MOUSEMOVE reports button state " << buttons
               << ", but internal state is " << _buttonState << std::endl;
#endif

    _buttonState = buttons;
}

bool EventHandler::_mouseButtonPress( const PointerButton button,
                                      const LPARAM lParam )
{
    PointerEvent pointerEvent;
    _buttonState |= button;
    pointerEvent.x = GET_X_LPARAM( lParam );
    pointerEvent.y = GET_Y_LPARAM( lParam );
    pointerEvent.buttons = _buttonState;
    pointerEvent.button = button;
    pointerEvent.modifiers = _getKeyModifiers();
    _computePointerDelta( EVENT_WINDOW_POINTER_BUTTON_PRESS, pointerEvent );
    return _window->processEvent( EVENT_WINDOW_POINTER_BUTTON_PRESS,
                                  pointerEvent );
}

bool EventHandler::_mouseButtonRelease( const PointerButton button,
                                        const LPARAM lParam )
{
    PointerEvent pointerEvent;
    _buttonState &= ~button;
    pointerEvent.x = GET_X_LPARAM( lParam );
    pointerEvent.y = GET_Y_LPARAM( lParam );
    pointerEvent.buttons = _buttonState;
    pointerEvent.button = button;
    pointerEvent.modifiers = _getKeyModifiers();
    _computePointerDelta( EVENT_WINDOW_POINTER_BUTTON_RELEASE, pointerEvent );
    return _window->processEvent( EVENT_WINDOW_POINTER_BUTTON_RELEASE,
                                  pointerEvent );
}

namespace
{
void _getWindowSize( HWND hWnd, SizeEvent& event )
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
LRESULT CALLBACK EventHandler::_wndProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                         LPARAM lParam )
{
    switch( uMsg )
    {
        case WM_SHOWWINDOW:
        {
            SizeEvent sizeEvent;
            _getWindowSize( hWnd, sizeEvent );

            EventType type =
                wParam == TRUE ? EVENT_WINDOW_SHOW : EVENT_WINDOW_HIDE;

            if( _window->processEvent( type, sizeEvent ))
                return TRUE;
            break;
        }

        case WM_CREATE:
        case WM_SIZE:
        case WM_MOVE:
        case WM_WINDOWPOSCHANGED:
        {
            SizeEvent sizeEvent;
            _getWindowSize( hWnd, sizeEvent );
            const bool hasArea = (sizeEvent.w >0 && sizeEvent.h > 0);
            const PixelViewport& pvp = _window->getPixelViewport();

            // No show/hide events on Win32?: Emulate.
            EventType type;
            if( !hasArea && pvp.hasArea( ))
                type = EVENT_WINDOW_HIDE;
            else if( hasArea && !pvp.hasArea( ))
                type = EVENT_WINDOW_SHOW;
            else
                type = EVENT_WINDOW_RESIZE;

            if( _window->processEvent( type, sizeEvent ))
                return TRUE;
            break;
        }

        case WM_CLOSE:
        case WM_DESTROY:
        {
            if( _window->processEvent( EVENT_WINDOW_CLOSE ))
                return TRUE;
            break;
        }

        case WM_PAINT:
        {
            if( GetUpdateRect( hWnd, 0, false ) == 0 ) // No 'expose'
                return DefWindowProc( hWnd, uMsg, wParam, lParam );

            if( _window->processEvent( EVENT_WINDOW_EXPOSE ))
                return TRUE;
            break;
        }

        case WM_MOUSEMOVE:
        {
            _syncButtonState( wParam );

            PointerEvent pointerEvent;
            pointerEvent.x = GET_X_LPARAM( lParam );
            pointerEvent.y = GET_Y_LPARAM( lParam );
            pointerEvent.buttons = _buttonState;

            _computePointerDelta( EVENT_WINDOW_POINTER_MOTION, pointerEvent );
            if( _window->processEvent( EVENT_WINDOW_POINTER_MOTION, pointerEvent ))
                return TRUE;
            break;
        }

        case WM_LBUTTONDOWN:
            if( _mouseButtonPress( PTR_BUTTON1, lParam ))
                return TRUE;
            break;

        case WM_RBUTTONDOWN:
            if( _mouseButtonPress( PTR_BUTTON2, lParam ))
                return TRUE;
            break;

        case WM_MBUTTONDOWN:
            if( _mouseButtonPress( PTR_BUTTON3, lParam ))
                return TRUE;
            break;

        case WM_XBUTTONDOWN:
        {
            const PointerButton button =
                GET_XBUTTON_WPARAM( wParam ) & XBUTTON1 ?
                                    PTR_BUTTON4 : PTR_BUTTON5;
            _syncButtonState( GET_XBUTTON_WPARAM( wParam ));
            if( _mouseButtonPress( button, lParam ))
                return TRUE;
            break;
        }

        case WM_LBUTTONUP:
            if( _mouseButtonRelease( PTR_BUTTON1, lParam ))
                return TRUE;
            break;

        case WM_RBUTTONUP:
            if( _mouseButtonRelease( PTR_BUTTON2, lParam ))
                return TRUE;
            break;

        case WM_MBUTTONUP:
            if( _mouseButtonRelease( PTR_BUTTON3, lParam ))
                return TRUE;
            break;

        case WM_XBUTTONUP:
        {
            const PointerButton button =
                GET_XBUTTON_WPARAM( wParam ) & XBUTTON1 ?
                                    PTR_BUTTON4 : PTR_BUTTON5;
            _syncButtonState( GET_XBUTTON_WPARAM( wParam ));
            if( _mouseButtonRelease( button, lParam ))
                return TRUE;
            break;
        }

        case WM_MOUSEWHEEL:
        {
            PointerEvent pointerEvent;
            pointerEvent.x     = GET_X_LPARAM( lParam );
            pointerEvent.y     = GET_Y_LPARAM( lParam );
            pointerEvent.buttons = _buttonState;
            pointerEvent.modifiers = _getKeyModifiers();
            pointerEvent.yAxis = _getWheelDelta( wParam );
            pointerEvent.button = PTR_BUTTON_NONE;
            _computePointerDelta( EVENT_WINDOW_POINTER_WHEEL, pointerEvent );
            if( _window->processEvent( EVENT_WINDOW_POINTER_WHEEL, pointerEvent ))
                return TRUE;
            break;
        }

#ifdef WM_MOUSEHWHEEL // only available on vista or later
        case WM_MOUSEHWHEEL:
        {
            PointerEvent pointerEvent;
            pointerEvent.x     = GET_X_LPARAM( lParam );
            pointerEvent.y     = GET_Y_LPARAM( lParam );
            pointerEvent.buttons = _buttonState;
            pointerEvent.modifiers = _getKeyModifiers();
            pointerEvent.xAxis = _getWheelDelta( wParam );
            pointerEvent.button = PTR_BUTTON_NONE;
            _computePointerDelta( EVENT_WINDOW_POINTER_WHEEL, pointerEvent );
            if( _window->processEvent( EVENT_WINDOW_POINTER_WHEEL, pointerEvent ))
                return TRUE;
            break;
        }
#endif

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            KeyEvent keyEvent;
            keyEvent.key = _getKey( lParam, wParam );
            keyEvent.modifiers = _getKeyModifiers();
            if( _window->processEvent( EVENT_KEY_PRESS, keyEvent ))
                return TRUE;
            break;
        }

        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            KeyEvent keyEvent;
            keyEvent.key = _getKey( lParam, wParam );
            keyEvent.modifiers = _getKeyModifiers();
            if( _window->processEvent( EVENT_KEY_RELEASE, keyEvent ))
                return TRUE;
            break;
        }

        case WM_SYSCOMMAND:
            switch( wParam )
            {
                case SC_MONITORPOWER:
                case SC_SCREENSAVE:
                    if( lParam >= 0 ) // request off
                    {
                        if( _window->processEvent( EVENT_WINDOW_SCREENSAVER ))
                            return TRUE;
                        break;
                    }
                    // else no break; fall through
                default:
                    _window->processEvent( EVENT_UNKNOWN );
                    LBVERB << "Unhandled system command 0x" << std::hex
                           << wParam  << std::dec << std::endl;
                    break;
            }
            break;
#ifdef EQUALIZER_USE_MAGELLAN
        case WM_INPUT:
            _magellanEventHandler( lParam );
            break;
#endif

        default:
            _window->processEvent( EVENT_UNKNOWN );
            LBVERB << "Unhandled message 0x" << std::hex << uMsg << std::dec
                   << std::endl;
            break;
    }

    return CallWindowProc( _prevWndProc, hWnd, uMsg, wParam, lParam );
}

int32_t EventHandler::_getWheelDelta( WPARAM wParam ) const
{
    const int32_t rawDelta =
        static_cast< int32_t >( GET_WHEEL_DELTA_WPARAM( wParam ));
    return rawDelta / _wheelDeltaPerLine;
}


#ifdef EQUALIZER_USE_MAGELLAN

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

void EventHandler::_magellanEventHandler( LPARAM lParam )
{
#ifdef EQUALIZER_USE_MAGELLAN
    RAWINPUTHEADER header;
    const UINT size_rawinputheader = sizeof( RAWINPUTHEADER );
    UINT size = sizeof(header);

    if( GetRawInputData( (HRAWINPUT)lParam, RID_HEADER, &header, &size,
                         size_rawinputheader ) == -1 )
    {
        LBINFO << "Error from GetRawInputData( RID_HEADER )" << std::endl;
        return;
    }

    // Ask Windows for the size of the event, because it is different on 32-bit
    // vs. 64-bit versions of the OS
    if( GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, 0, &size,
                         size_rawinputheader ) == -1)
    {
        LBINFO << "Error from GetRawInputData(RID_INPUT)" << std::endl;
        return;
    }

    // Set aside enough memory for the full event
    LPRAWINPUT evt = (LPRAWINPUT)alloca( size );

    if( GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, evt, &size,
                         size_rawinputheader ) == -1)
    {
        LBINFO << "Error from GetRawInputData(RID_INPUT)" << std::endl;
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
            ButtonEvent event;
            LBASSERT( _magellanNode->getID() != 0 );
            event.button = 0;
            event.buttons = 0;
            // TODO: find fired button(s) since last buttons event

            if( pRawHid->bRawData[1] )
                event.buttons = PTR_BUTTON1;
            if( pRawHid->bRawData[2] )
                event.buttons |= PTR_BUTTON2;
            if( pRawHid->bRawData[3] )
                event.buttons |= PTR_BUTTON3;
            _magellanNode->processEvent( event );
        }
        else
        {
            LBASSERTINFO( 0, "Unimplemented space mouse command " <<
                          pRawHid->bRawData[0] );
        }

        if (_magellanGotTranslation && _magellanGotRotation)
        {
            AxisEvent event;
            LBASSERT( _magellanNode->getID() != 0 );
            event.xAxis =  _magellanDOFs[0];
            event.yAxis = -_magellanDOFs[1];
            event.zAxis = -_magellanDOFs[2];
            event.xRotation = -_magellanDOFs[3];
            event.yRotation =  _magellanDOFs[4];
            event.zRotation =  _magellanDOFs[5];

            _magellanGotRotation = false;
            _magellanGotTranslation = false;
            _magellanNode->processEvent( event );
        }
    }
#endif
}

bool EventHandler::initMagellan( Node* node )
{
#ifdef EQUALIZER_USE_MAGELLAN
    _magellanGotRotation = false;
    _magellanGotTranslation = false;

    // Find the Raw Devices
    UINT nDevices;

    // Get Number of devices attached
    if( GetRawInputDeviceList( 0, &nDevices, sizeof( RAWINPUTDEVICELIST )) != 0)
    {
        LBINFO << "No RawInput devices attached" << std::endl;
        return false;
    }
    // Create list large enough to hold all RAWINPUTDEVICE structs
    _pRawInputDeviceList = (PRAWINPUTDEVICELIST)malloc(
        sizeof( RAWINPUTDEVICELIST ) * nDevices );
    if( !_pRawInputDeviceList )
    {
        LBINFO << "Error mallocing RAWINPUTDEVICELIST" << std::endl;
        return false;
    }
    // Now get the data on the attached devices
    if( GetRawInputDeviceList( _pRawInputDeviceList, &nDevices,
                               sizeof( RAWINPUTDEVICELIST )) == -1)
    {
        LBINFO << "Error from GetRawInputDeviceList" << std::endl;
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
                LBINFO << "Device [" << i << "]: handle="
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
                    LBINFO << "VID = " << phidInfo->dwVendorId << std::endl
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
        LBVERB << "Error calling RegisterRawInputDevices" << std::endl;
        return false;
    }

    LBINFO << "Found and connected." << std::endl;
     _magellanNode = node;
#endif
    return true;
}

void EventHandler::exitMagellan(eq::Node *node)
{
#ifdef EQUALIZER_USE_MAGELLAN
    if( _magellanNode == node )
    {
        free(_pRawInputDeviceList);
        _magellanNode = 0;
    }
#endif
}

}
}
