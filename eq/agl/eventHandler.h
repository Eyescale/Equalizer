
/* Copyright (c) 2007-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_AGL_EVENTHANDLER_H
#define EQ_AGL_EVENTHANDLER_H

#include <eq/defines.h>
#include <eq/os.h>
#ifdef AGL
#include <eq/agl/types.h>
#include <eq/eventHandler.h> // base class

namespace eq
{
namespace agl
{
/**
 * The event handler for AGL windows.
 *
 * Any implementation of the agl::WindowIF can instantiate this event handler,
 * which registers for Carbon events, translates each received event to an
 * AGLWindowEvent and dispatches it to agl::WindowIF::processEvent.
 */
class EventHandler : public eq::EventHandler
{
public:
    /**
     * Construct a new AGL event handler for the given AGL window.
     * @version 1.0
     */
    EventHandler( agl::WindowIF* window );

    /** Destruct the AGL event handler. @version 1.0 */
    virtual ~EventHandler();

    /**
     * Initialize space mouse event handling for this process.
     *
     * Received space mouse events are processed by Node::processEvent().
     * @version 1.0
     */
    static void initMagellan( Node* node );

    /**
     * De-initialize space mouse event handling for this process.
     * @sa Node::configInit
     * @version 1.0
     */
    static void exitMagellan( Node* node );

    /** @return the handled AGL window. @version 1.0 */
    agl::WindowIF* getWindow() const { return _window; }

    /** @internal */
    bool handleEvent( EventRef event );

private:
    agl::WindowIF* const _window;

    EventHandlerRef _eventHandler;
    EventHandlerRef _eventDispatcher;

    void _processWindowEvent( WindowEvent& event );
    /** @return true if the event is valid for the window. */
    bool _processMouseEvent( WindowEvent& event );
    void _processKeyEvent( WindowEvent& event );

    uint32_t _getButtonState();
    uint32_t _getButtonAction( EventRef event );
    uint32_t _getKey( EventRef event );

    uint32_t _lastDX;
    uint32_t _lastDY;
};
}
}
#endif // AGL
#endif // EQ_AGL_EVENTHANDLER_H
