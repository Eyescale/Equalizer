
/* Copyright (c) 2014-2016, Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Juan Hernando <jhernando@fi.upm.es>
 *                          Stefan.Eilemann@epfl.ch
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

#include "event.h"
#include "window.h"
#include <eq/fabric/keyEvent.h>
#include <eq/fabric/sizeEvent.h>

#include <QApplication>
#include <QKeyEvent>
#include <QThread>

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

KeyModifier _getKeyModifiers( const QInputEvent& event )
{
    Qt::KeyboardModifiers modifiers = event.modifiers();
    KeyModifier result = KeyModifier::none;
    if( modifiers & Qt::AltModifier )
        result |= KeyModifier::alt;
    if( modifiers & Qt::ControlModifier )
        result |= KeyModifier::control;
    if( modifiers & Qt::ShiftModifier )
        result |= KeyModifier::shift;
    return result;
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

// Performs a (derived) copy of the incoming event. QApplication::postEvent()
// takes ownership of the given event, and the incoming event is only valid
// within the event function, it is destroyed afterwards by Qt.
QEvent* _clone( const QEvent* event )
{
    switch( event->type( ))
    {
    case QEvent::Close:
        return new QCloseEvent;
    case QEvent::Expose:
    {
        const QExposeEvent* e = static_cast< const QExposeEvent* >( event );
        return new QExposeEvent( e->region( ));
    }
    case QEvent::Hide:
        return new QHideEvent;
    case QEvent::Resize:
    {
        const QResizeEvent* e = static_cast< const QResizeEvent* >( event );
        return new QResizeEvent( e->size(), e->oldSize( ));
    }
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    {
        const QMouseEvent* e = static_cast< const QMouseEvent* >( event );
        return new QMouseEvent( e->type(), e->localPos(), e->windowPos(),
                                e->screenPos(), e->button(), e->buttons(),
                                e->modifiers( ));
    }
    case QEvent::Wheel:
    {
        const QWheelEvent* e = static_cast< const QWheelEvent* >( event );
        return new QWheelEvent( e->pos(), e->globalPos(), e->pixelDelta(),
                                e->angleDelta(), e->delta(), e->orientation(),
                                e->buttons(), e->modifiers(), e->phase()
#if QT_VERSION >= 0x050500
                                , e->source()
#endif
                                );
    }
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    {
        const QKeyEvent* e = static_cast< const QKeyEvent* >( event );
        return new QKeyEvent( e->type(), e->key(), e->modifiers(),
                              e->nativeScanCode(), e->nativeVirtualKey(),
                              e->nativeModifiers(), e->text(),
                              e->isAutoRepeat(), ushort( e->count( )));
    }
    default:
        return nullptr;
    }
}
}

EventHandler::EventHandler( WindowIF& window )
    : _window( window )
{}

EventHandler::~EventHandler()
{}

bool EventHandler::event( QEvent* qevent )
{
    if( thread() != QThread::currentThread( ))
    {
        // Event did not arrive in the current thread, repost to correct thread.
        QEvent* clone = _clone( qevent );
        if( clone )
            QApplication::postEvent( this, clone );
        // else not cloned because unknown/not handled by us later (see below)

        return true;
    }

    return _processEvent( qevent );
}

bool EventHandler::_processEvent( QEvent* qEvent )
{
    switch( qEvent->type( ))
    {
    case QEvent::Close:
        return _window.processEvent( EVENT_WINDOW_CLOSE, qEvent );
    case QEvent::Expose:
        return _window.processEvent( EVENT_WINDOW_EXPOSE, qEvent );
    case QEvent::Hide:
        return _window.processEvent( EVENT_WINDOW_HIDE, qEvent );
    case QEvent::Resize:
        return _processSizeEvent( EVENT_WINDOW_RESIZE, qEvent );
    case QEvent::MouseMove:
        return _processPointerEvent( EVENT_WINDOW_POINTER_MOTION, qEvent );
    case QEvent::MouseButtonPress:
        return _processPointerEvent( EVENT_WINDOW_POINTER_BUTTON_PRESS,
                                     qEvent );
    case QEvent::MouseButtonRelease:
        return _processPointerEvent( EVENT_WINDOW_POINTER_BUTTON_RELEASE,
                                     qEvent );
#ifndef QT_NO_WHEELEVENT
    case QEvent::Wheel:
        return _processWheelEvent( qEvent );
#endif
    case QEvent::KeyPress:
        return _processKeyEvent( EVENT_KEY_PRESS, qEvent );
    case QEvent::KeyRelease:
        return _processKeyEvent( EVENT_KEY_RELEASE, qEvent );
    default:
        return false;
    }
}

bool EventHandler::_processSizeEvent( const EventType type, QEvent* qev )
{
    QResizeEvent* qevent = static_cast< QResizeEvent* >( qev );
    SizeEvent sizeEvent;
    sizeEvent.w = qevent->size().width();
    sizeEvent.h = qevent->size().height();
    return _window.processEvent( type, qev, sizeEvent );
}

bool EventHandler::_processPointerEvent( const EventType type, QEvent* qev )
{
    QMouseEvent* qevent = static_cast< QMouseEvent* >( qev );
    PointerEvent pointerEvent;
    pointerEvent.x = qevent->x();
    pointerEvent.y = qevent->y();
    pointerEvent.buttons = _getButtons( qevent->buttons( ));
    pointerEvent.button = _getButton( qevent->button( ));
    pointerEvent.modifiers = _getKeyModifiers( *qevent );
    _computePointerDelta( type, pointerEvent );
    return _window.processEvent( type, qev, pointerEvent );
}

#ifndef QT_NO_WHEELEVENT
bool EventHandler::_processWheelEvent( QEvent* qev )
{
    QWheelEvent* qevent = static_cast< QWheelEvent* >( qev );
    PointerEvent pointerEvent;
    switch( qevent->orientation( ))
    {
    case Qt::Horizontal:
        pointerEvent.xAxis = qevent->delta() > 0 ? 1 : -1;
        break;
    case Qt::Vertical:
        pointerEvent.yAxis = qevent->delta() > 0 ? 1 : -1;
        break;
    }
    pointerEvent.buttons = _getButtons( qevent->buttons( ));
    pointerEvent.button  = PTR_BUTTON_NONE;
    pointerEvent.modifiers = _getKeyModifiers( *qevent );
    return _window.processEvent( EVENT_WINDOW_POINTER_WHEEL, qev, pointerEvent);
}
#endif

bool EventHandler::_processKeyEvent( const EventType type, QEvent* qev )
{
    QKeyEvent* qevent = static_cast< QKeyEvent* >( qev );
    KeyEvent keyEvent;
    keyEvent.key = _getKey( *qevent );
    keyEvent.modifiers = _getKeyModifiers( *qevent );
    return _window.processEvent( type, qev, keyEvent );
}

}
}
