
/* Copyright (c) 2009-2015, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_OBSERVER_H
#define EQ_OBSERVER_H

#include <eq/api.h>
#include <eq/types.h>
#include <eq/fabric/observer.h>         // base class

namespace eq
{
namespace detail { class Observer; }

/**
 * An Observer looks at one or more views from a certain position (head matrix)
 * with a given eye separation. Multiple observers in a configuration can be
 * used to update independent viewers from one configuration, e.g., a control
 * host, a HMD and a Cave.
 *
 * @sa fabric::Observer
 */
class Observer : public fabric::Observer< Config, Observer >
{
public:
    /** Construct a new observer. @version 1.0 */
    EQ_API explicit Observer( Config* parent );

    /** Destruct this observer. @version 1.0 */
    EQ_API virtual ~Observer();

    /** @name Operations */
    //@{
    /**
     * Handle an event.
     *
     * The event type and originator identifier (of this object) have
     * already been consumed from the given command.
     *
     * @param command The event input command.
     * @return true if the event requires a redraw, false otherwise.
     */
    EQ_API virtual bool handleEvent( EventICommand& command );
    //@}

    /** @name Data Access */
    //@{
    /** @return the Server of this observer. @version 1.0 */
    EQ_API ServerPtr getServer();
    //@}

    void addView( View* ) { /* nop */ } //!< @internal
    void removeView( View* ) { /* nop */ } //!< @internal

protected:
    /**
     * @name Callbacks
     *
     * Callbacks are called by Equalizer during rendering to execute various
     * actions from the application main thread before sending the
     * corresponding command to the server.
     */
    //@{
    /** Initialize this observer. @version 1.5.2 */
    EQ_API virtual bool configInit();
    friend class detail::InitVisitor;

    /** Exit this observer. @version 1.5.2 */
    EQ_API virtual bool configExit();
    friend class detail::ExitVisitor;

    /**
     * Start rendering a frame.
     *
     * Called once at the beginning of each frame before the Config::frame
     * is send to the server, to do per-frame updates of observer-specific
     * data.
     *
     * @param frameNumber the frame to start.
     * @version 1.5.2
     */
    EQ_API virtual void frameStart( const uint32_t frameNumber );
    friend class detail::FrameVisitor;
    //@}

private:
    detail::Observer* const _impl;
};
}
#endif // EQ_OBSERVER_H
