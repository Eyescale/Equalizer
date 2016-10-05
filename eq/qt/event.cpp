
/* Copyright (c) 2016, Stefan.Eilemann@epfl.ch
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
#include "event.h"

namespace eq
{
namespace qt
{
namespace
{
static EventType _getType( const QEvent* from )
{
    switch( event->type( ))
    {
    case QEvent::Expose: return EVENT_WINDOW_EXPOSE;
    case QEvent::Hide: return EVENT_WINDOW_HIDE;
    case QEvent::Resize: return EVENT_WINDOW_RESIZE;
    case QEvent::Close: return EVENT_WINDOW_CLOSE;
    case QEvent::MouseMove: return EVENT_WINDOW_POINTER_MOTION;
    case QEvent::MouseButtonPress: return EVENT_WINDOW_POINTER_BUTTON_PRESS;
    case QEvent::MouseButtonRelease: return EVENT_WINDOW_POINTER_BUTTON_RELEASE;
    case QEvent::KeyPress: return EVENT_KEY_PRESS;
    case QEvent::KeyRelease: return EVENT_KEY_RELEASE;
#ifndef QT_NO_WHEELEVENT
    case QEvent::Wheel: return EVENT_WINDOW_POINTER_WHEEL;
#endif
    default: return EVENT_UNKNOWN;
    }
}

uint32_t _getKey( const QKeyEvent& keyEvent )
{
    switch( keyEvent.key( ))
    {
    case Qt::Key_Escape:    return KC_ESCAPE;
    case Qt::Key_Backspace: return KC_BACKSPACE;
    case Qt::Key_Return:    return KC_RETURN;
    case Qt::Key_Tab:       return KC_TAB;
    case Qt::Key_Home:      return KC_HOME;
    case Qt::Key_Left:      return KC_LEFT;
    case Qt::Key_Up:        return KC_UP;
    case Qt::Key_Right:     return KC_RIGHT;
    case Qt::Key_Down:      return KC_DOWN;
    case Qt::Key_PageUp:    return KC_PAGE_UP;
    case Qt::Key_PageDown:  return KC_PAGE_DOWN;
    case Qt::Key_End:       return KC_END;
    case Qt::Key_F1:        return KC_F1;
    case Qt::Key_F2:        return KC_F2;
    case Qt::Key_F3:        return KC_F3;
    case Qt::Key_F4:        return KC_F4;
    case Qt::Key_F5:        return KC_F5;
    case Qt::Key_F6:        return KC_F6;
    case Qt::Key_F7:        return KC_F7;
    case Qt::Key_F8:        return KC_F8;
    case Qt::Key_F9:        return KC_F9;
    case Qt::Key_F10:       return KC_F10;
    case Qt::Key_F11:       return KC_F11;
    case Qt::Key_F12:       return KC_F12;
    case Qt::Key_F13:       return KC_F13;
    case Qt::Key_F14:       return KC_F14;
    case Qt::Key_F15:       return KC_F15;
    case Qt::Key_F16:       return KC_F16;
    case Qt::Key_F17:       return KC_F17;
    case Qt::Key_F18:       return KC_F18;
    case Qt::Key_F19:       return KC_F19;
    case Qt::Key_F20:       return KC_F20;
    case Qt::Key_F21:       return KC_F21;
    case Qt::Key_F22:       return KC_F22;
    case Qt::Key_F23:       return KC_F23;
    case Qt::Key_F24:       return KC_F24;
    case Qt::Key_Shift:     return KC_SHIFT_L;
    case Qt::Key_Control:   return KC_CONTROL_L;
    case Qt::Key_Alt:       return KC_ALT_L;
    case Qt::Key_AltGr:     return KC_ALT_R;
    case Qt::Key_unknown:   return KC_VOID;
    default:
        if( keyEvent.text().isEmpty( ))
            return KC_VOID;
        return keyEvent.text().at( 0 ).unicode();
    }
}

// Qt buttons 2 & 3 are inversed with EQ (X11/AGL/WGL)
uint32_t _getButtons( const Qt::MouseButtons& eventButtons )
{
    if( (eventButtons & (Qt::MidButton | Qt::RightButton)) ==
                                             (Qt::MidButton | Qt::RightButton) )
    {
        return eventButtons;
    }

    uint32_t buttons = eventButtons;
    if( eventButtons & Qt::MidButton || eventButtons & Qt::RightButton )
        buttons ^= PTR_BUTTON2 | PTR_BUTTON3;
    return buttons;
}

uint32_t _getButton( const Qt::MouseButton button )
{
    if( button == Qt::RightButton )
        return PTR_BUTTON2;
    if( button == Qt::MidButton )
        return PTR_BUTTON3;
    return button;
}
}

template<> Event< eq::Event >::Event( const QEvent* from )
    : type( _getType( from ))
{}

template<> Event< eq::SizeEvent >::Event( const QEvent* from )
    : type( _getType( from ))
{
    const QResizeEvent* qevent = static_cast< const QResizeEvent* >( from );
    event.w = qevent->size().width();
    event.h = qevent->size().height();
}

template<> Event< eq::PointerEvent >::Event( const QEvent* from )
    : type( _getType( from ))
{
#ifndef QT_NO_WHEELEVENT
    if( type == EVENT_WINDOW_POINTER_WHEEL )
    {
        const QWheelEvent* qevent = static_cast< const QWheelEvent* >( from );
        PointerEvent event;
        switch( qevent->orientation( ))
        {
        case Qt::Horizontal:
            event.xAxis = qevent->delta() > 0 ? 1 : -1;
            break;
        case Qt::Vertical:
            event.yAxis = qevent->delta() > 0 ? 1 : -1;
            break;
        }
        event.buttons = _getButtons( qevent->buttons( ));
        event.button  = PTR_BUTTON_NONE;
    }
    else
#endif
    {
        const QMouseEvent* qevent = static_cast< const QMouseEvent* >( from );
        event.x = qevent->x();
        event.y = qevent->y();
        event.buttons = _getButtons( qevent->buttons( ));
        event.button = _getButton( qevent->button( ));
    }
}

template<> Event< eq::KeyEvent >::Event( const QEvent* from )
    : type( _getType( from ))
{
    const QKeyEvent* qevent = static_cast< const QKeyEvent* >( from );
    event.key = _getKey( *qevent );
}

}
}

#endif
