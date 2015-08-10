
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

#ifndef EQ_QT_EVENTHANDLER_H
#define EQ_QT_EVENTHANDLER_H

#include <eq/qt/types.h>
#include <eq/eventHandler.h> // base class
#include <QObject> // base class

namespace eq
{
namespace qt
{

/** The event handler for Qt windows.
 *
 * Reposts Qt window, mouse and keyboard events to itself in the shape of a Qt
 * user event that contains an Equalizer event.
*/
class EventHandler : public QObject, public eq::EventHandler
{
public:
    /** Construct a new Qt event handler. @version 1.7.3 */
    explicit EventHandler( WindowIF& window );

    /** Destruct the Qt event handler. @version  1.7.3 */
    ~EventHandler() final;

    void exposeEvent();
    void hideEvent();
    void resizeEvent( QResizeEvent* );
    void closeEvent();

    void mousePressEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );
    void wheelEvent( QWheelEvent* );
    void keyPressEvent( QKeyEvent* );
    void keyReleaseEvent( QKeyEvent* );

private:
    WindowIF& _window;

    bool event( QEvent* evt ) override;

    WindowEvent* _translateEvent( QEvent* );
    WindowEvent* _simpleWindowEvent( eq::Event::Type type );
    WindowEvent* _resizeEvent( QResizeEvent* );
    WindowEvent* _mousePressEvent( QMouseEvent* );
    WindowEvent* _mouseReleaseEvent( QMouseEvent* );
    WindowEvent* _mouseMoveEvent( QMouseEvent* );
    WindowEvent* _wheelEvent( QWheelEvent* );
    WindowEvent* _keyEvent( QKeyEvent*, eq::Event::Type type );
};
}
}
#endif // EQ_QT_EVENTHANDLER_H
