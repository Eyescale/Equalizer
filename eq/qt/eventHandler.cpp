
/* Copyright (c) 2014, Daniel Nachbaur <danielnachbaur@gmail.com>
 *               2015, Juan Hernando <jhernando@fi.upm.es>
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

#include "eventHandler.h"

#include "window.h"
#include "windowEvent.h"

#include <QApplication>
#include <QKeyEvent>

namespace eq
{
namespace qt
{

namespace
{
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

EventHandler::EventHandler( WindowIF& window )
    : _window( window )
{
}

EventHandler::~EventHandler()
{
}

void EventHandler::exposeEvent()
{
    QApplication::postEvent( this, _simpleWindowEvent( Event::WINDOW_EXPOSE ));
}

void EventHandler::hideEvent()
{
    QApplication::postEvent( this, _simpleWindowEvent( Event::WINDOW_HIDE ));
}

void EventHandler::resizeEvent( QResizeEvent* qevent )
{
    QApplication::postEvent( this, _resizeEvent( qevent ));
}

void EventHandler::closeEvent()
{
    QApplication::postEvent( this, _simpleWindowEvent( Event::WINDOW_CLOSE ));
}

void EventHandler::mousePressEvent( QMouseEvent* qevent )
{
    QApplication::postEvent( this, _mousePressEvent( qevent ));
}

void EventHandler::mouseReleaseEvent( QMouseEvent* qevent )
{
    QApplication::postEvent( this, _mouseReleaseEvent( qevent ));
}

void EventHandler::mouseMoveEvent( QMouseEvent* qevent )
{
    QApplication::postEvent( this, _mouseMoveEvent( qevent ));
}

#ifndef QT_NO_WHEELEVENT
void EventHandler::wheelEvent( QWheelEvent* qevent )
{
    QApplication::postEvent( this, _wheelEvent( qevent ));
}
#endif

void EventHandler::keyPressEvent( QKeyEvent* qevent )
{
    QApplication::postEvent( this, _keyEvent( qevent, Event::KEY_PRESS ));
}

void EventHandler::keyReleaseEvent( QKeyEvent* qevent )
{
    QApplication::postEvent( this, _keyEvent( qevent, Event::KEY_RELEASE ));
}

bool EventHandler::event( QEvent* qevent )
{
    if( qevent->type() != QEvent::User )
    {
        // This event is coming directly into this object from the GUI thread.
        // Reposting with the supported type and to ensure that the correct
        // thread processes it.
        WindowEvent* windowEvent = _translateEvent( qevent );
        if( windowEvent )
        {
            QApplication::postEvent( this, windowEvent );
            return true;
        }
        return false;
    }

    WindowEvent* windowEvent = dynamic_cast< WindowEvent* >( qevent );
    if( !windowEvent )
        return false;

    switch( windowEvent->eq::Event::type )
    {
    case Event::WINDOW_POINTER_MOTION:
    case Event::WINDOW_POINTER_BUTTON_PRESS:
    case Event::WINDOW_POINTER_BUTTON_RELEASE:
        _computePointerDelta( *windowEvent );
        break;
    default:
        break;
    }

    return _window.processEvent( *windowEvent );
}

WindowEvent* EventHandler::_translateEvent( QEvent* qevent )
{
    switch( qevent->type( ))
    {
    case QEvent::Expose:
        return _simpleWindowEvent( Event::WINDOW_EXPOSE );
    case QEvent::Hide:
        return _simpleWindowEvent( Event::WINDOW_HIDE );
    case QEvent::Resize:
        return _resizeEvent( static_cast< QResizeEvent* >( qevent ));
    case QEvent::Close:
        return _simpleWindowEvent( Event::WINDOW_CLOSE );
    case QEvent::MouseMove:
        return _mouseMoveEvent( static_cast< QMouseEvent* >( qevent ));
    case QEvent::MouseButtonPress:
        return _mousePressEvent( static_cast< QMouseEvent* >( qevent ));
    case QEvent::MouseButtonRelease:
        return _mouseReleaseEvent( static_cast< QMouseEvent* >( qevent ));
#ifndef QT_NO_WHEELEVENT
    case QEvent::Wheel:
        return _wheelEvent( static_cast< QWheelEvent* >( qevent ));
#endif
    case QEvent::KeyPress:
        return _keyEvent( static_cast< QKeyEvent* >( qevent ),
                          Event::KEY_PRESS);
    case QEvent::KeyRelease:
        return _keyEvent( static_cast< QKeyEvent* >( qevent ),
                          Event::KEY_RELEASE );
    default:
        ;
    }
    return 0;
}

WindowEvent* EventHandler::_simpleWindowEvent( const eq::Event::Type type )
{
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = type;
    return windowEvent;
}

WindowEvent* EventHandler::_resizeEvent( QResizeEvent* qevent )
{
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::WINDOW_RESIZE;
    windowEvent->resize.w = qevent->size().width();
    windowEvent->resize.h = qevent->size().height();
    return windowEvent;
}

WindowEvent* EventHandler::_mousePressEvent( QMouseEvent* qevent )
{
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::WINDOW_POINTER_BUTTON_PRESS;
    windowEvent->pointerButtonPress.x = qevent->x();
    windowEvent->pointerButtonPress.y = qevent->y();
    windowEvent->pointerButtonPress.buttons = _getButtons( qevent->buttons( ));
    windowEvent->pointerButtonPress.button  = _getButton( qevent->button( ));
    return windowEvent;
}

WindowEvent* EventHandler::_mouseReleaseEvent( QMouseEvent* qevent )
{
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::WINDOW_POINTER_BUTTON_RELEASE;
    windowEvent->pointerButtonRelease.x = qevent->x();
    windowEvent->pointerButtonRelease.y = qevent->y();
    windowEvent->pointerButtonRelease.buttons = _getButtons( qevent->buttons());
    windowEvent->pointerButtonRelease.button  = _getButton( qevent->button( ));
    return windowEvent;
}

WindowEvent* EventHandler::_mouseMoveEvent( QMouseEvent* qevent )
{
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::WINDOW_POINTER_MOTION;
    windowEvent->pointerMotion.x = qevent->x();
    windowEvent->pointerMotion.y = qevent->y();
    windowEvent->pointerMotion.buttons = _getButtons( qevent->buttons( ));
    windowEvent->pointerMotion.button  = _getButton( qevent->button( ));
    return windowEvent;
}

#ifndef QT_NO_WHEELEVENT
WindowEvent* EventHandler::_wheelEvent( QWheelEvent* qevent )
{
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::WINDOW_POINTER_WHEEL;
    switch( qevent->orientation( ))
    {
    case Qt::Horizontal:
        windowEvent->pointerWheel.xAxis = qevent->delta() > 0 ? 1 : -1;
        break;
    case Qt::Vertical:
        windowEvent->pointerWheel.yAxis = qevent->delta() > 0 ? 1 : -1;
        break;
    }
    windowEvent->pointerWheel.buttons = _getButtons( qevent->buttons( ));
    windowEvent->pointerWheel.button  = PTR_BUTTON_NONE;
    return windowEvent;
}
#endif

WindowEvent* EventHandler::_keyEvent( QKeyEvent* qevent,
                                      const eq::Event::Type type )
{
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = type;
    windowEvent->keyPress.key = _getKey( *qevent );
    return windowEvent;
}

}
}
