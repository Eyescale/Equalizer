
/* Copyright (c) 2014-2016, Daniel Nachbaur <danielnachbaur@gmail.com>
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
 * If needed, reposts Qt events to the thread which called the constructor
 * before processing them.
 */
class EventHandler : public QObject, public eq::EventHandler
{
public:
    /** Construct a new Qt event handler. @version 1.7.3 */
    explicit EventHandler( WindowIF& window );

    /** Destruct the Qt event handler. @version  1.7.3 */
    ~EventHandler() final;

    /** @return the handled parent window */
    WindowIF& getWindow() { return _window; }

private:
    WindowIF& _window;

    bool event( QEvent* evt ) override;

    bool _processEvent( QEvent* event );
    bool _processSizeEvent( EventType, QEvent* );
    bool _processPointerEvent( EventType, QEvent* );
    bool _processWheelEvent( QEvent* );
    bool _processKeyEvent( EventType, QEvent* );
};
}
}
#endif // EQ_QT_EVENTHANDLER_H
