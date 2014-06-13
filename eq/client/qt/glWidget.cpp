
/* Copyright (c) 2014, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "eventHandler.h" // be first to avoid max/min name clashes on Win32...

#include "glWidget.h"

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
uint32_t _getKey( const QKeyEvent keyEvent )
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
}

GLWidget::GLWidget( const QGLFormat& format_, const GLWidget* shareWidget )
    : QGLWidget( format_, 0, shareWidget )
    , _parent( 0 )
    , _eventHandler( 0 )
{
    setAutoBufferSwap( false );
}

GLWidget::~GLWidget()
{
    LBASSERT( !_eventHandler );
}

void GLWidget::setParent( qt::Window* parent_ )
{
    LBASSERT( !_parent && parent_ );
    _parent = parent_;
}

void GLWidget::initEventHandler()
{
    if( _parent )
        _eventHandler = new EventHandler( *_parent );
}

void GLWidget::exitEventHandler()
{
    delete _eventHandler;
    _eventHandler = 0;
}

void GLWidget::resizeEvent( QResizeEvent* qevent )
{
    if( !_eventHandler )
        return;
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::WINDOW_RESIZE;
    windowEvent->resize.w = qevent->size().width();
    windowEvent->resize.h = qevent->size().height();
    QApplication::postEvent( _eventHandler, windowEvent );
}

void GLWidget::closeEvent( QCloseEvent* )
{
    if( !_eventHandler )
        return;
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::WINDOW_CLOSE;
    QApplication::postEvent( _eventHandler, windowEvent );
}

void GLWidget::paintEvent( QPaintEvent* )
{
    if( !_eventHandler )
        return;
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::WINDOW_EXPOSE;
    QApplication::postEvent( _eventHandler, windowEvent );
}

void GLWidget::mousePressEvent( QMouseEvent* qevent )
{
    if( !_eventHandler )
        return;
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::WINDOW_POINTER_BUTTON_PRESS;
    windowEvent->pointerButtonPress.x = qevent->x();
    windowEvent->pointerButtonPress.y = qevent->y();
    windowEvent->pointerButtonPress.buttons = qevent->buttons();
    windowEvent->pointerButtonPress.button  = qevent->button();
    QApplication::postEvent( _eventHandler, windowEvent );
}

void GLWidget::mouseReleaseEvent( QMouseEvent* qevent )
{
    if( !_eventHandler )
        return;
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::WINDOW_POINTER_BUTTON_RELEASE;
    windowEvent->pointerButtonRelease.x = qevent->x();
    windowEvent->pointerButtonRelease.y = qevent->y();
    windowEvent->pointerButtonRelease.buttons = qevent->buttons();
    windowEvent->pointerButtonRelease.button  = qevent->button();
    QApplication::postEvent( _eventHandler, windowEvent );
}

void GLWidget::mouseMoveEvent( QMouseEvent* qevent )
{
    if( !_eventHandler )
        return;
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::WINDOW_POINTER_MOTION;
    windowEvent->pointerMotion.x = qevent->x();
    windowEvent->pointerMotion.y = qevent->y();
    windowEvent->pointerMotion.buttons = qevent->buttons();
    windowEvent->pointerMotion.button  = qevent->button();
    QApplication::postEvent( _eventHandler, windowEvent );
}

#ifndef QT_NO_WHEELEVENT
void GLWidget::wheelEvent( QWheelEvent* qevent )
{
    if( !_eventHandler )
        return;
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::WINDOW_POINTER_WHEEL;
    switch( qevent->orientation( ))
    {
    case Qt::Horizontal:
        windowEvent->pointerWheel.yAxis = qevent->delta() > 0 ? 1 : -1;
        break;
    case Qt::Vertical:
        windowEvent->pointerWheel.xAxis = qevent->delta() > 0 ? 1 : -1;
        break;
    }
    windowEvent->pointerWheel.buttons = qevent->buttons();
    windowEvent->pointerWheel.button  = PTR_BUTTON_NONE;
    QApplication::postEvent( _eventHandler, windowEvent );
}
#endif

void GLWidget::keyPressEvent( QKeyEvent* qevent )
{
    if( !_eventHandler )
        return;
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::KEY_PRESS;
    windowEvent->keyPress.key = _getKey( *qevent );
    QApplication::postEvent( _eventHandler, windowEvent );
}

void GLWidget::keyReleaseEvent( QKeyEvent* qevent )
{
    if( !_eventHandler )
        return;
    WindowEvent* windowEvent = new WindowEvent;
    windowEvent->eq::Event::type = Event::KEY_RELEASE;
    windowEvent->keyRelease.key = _getKey( *qevent );
    QApplication::postEvent( _eventHandler, windowEvent );
}

}
}
