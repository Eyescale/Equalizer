
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQSERVER_PIPE_H
#define EQSERVER_PIPE_H

#include <eq/server/api.h>
#include "types.h"
#include "state.h"         // enum
#include "visitorResult.h" // enum

#include <eq/fabric/iAttribute.h>       // eq::OFF enum
#include <eq/fabric/pipe.h>             // parent
#include <eq/fabric/pixelViewport.h>    // member
#include <lunchbox/monitor.h>            // member

#include <ostream>
#include <vector>

namespace eq
{
namespace server
{
/** The pipe. */
class Pipe : public fabric::Pipe< Node, Pipe, Window, PipeVisitor >
{
public:
    /** Construct a new Pipe. */
    EQSERVER_API explicit Pipe( Node* parent );

    virtual ~Pipe();

    ServerPtr getServer();
    ConstServerPtr getServer() const;

    Config* getConfig();
    const Config* getConfig() const;

    co::CommandQueue* getMainThreadQueue();
    co::CommandQueue* getCommandThreadQueue();

    Channel* getChannel( const ChannelPath& path );

    /** @return the state of this pipe. */
    State getState()    const { return _state.get(); }

    /** @internal */
    void setState( const State state ) { _state = state; }

    /** Increase pipe activition count. */
    void activate();

    /** Decrease pipe activition count. */
    void deactivate();

    /** @return if this pipe is actively used for rendering. */
    bool isActive() const { return (_active != 0); }

    /** @return if this pipe is running. */
    bool isRunning() const { return _state == STATE_RUNNING; }

    /**
     * Add additional tasks this pipe, and all its parents, might
     * potentially execute.
     */
    void addTasks( const uint32_t tasks );

    /**
     * @name Data Access
     */
    //@{
    /** The last drawing compound for this entity. @internal */
    void setLastDrawWindow( const Window* window )
    { _lastDrawWindow = window; }
    const Window* getLastDrawWindow() const { return _lastDrawWindow; }
    //@}

    /**
     * @name Operations
     */
    //@{
    /** Start initializing this entity. */
    void configInit( const uint128_t& initID, const uint32_t frameNumber );

    /** Sync initialization of this entity. */
    bool syncConfigInit();

    /** Start exiting this entity. */
    void configExit();

    /** Sync exit of this entity. */
    bool syncConfigExit();

    /**
     * Trigger the rendering of a new frame.
     *
     * @param frameID a per-frame identifier passed to all rendering
     *                methods.
     * @param frameNumber the number of the frame.
     */
    void update( const uint128_t& frameID, const uint32_t frameNumber );
    //@}

    co::ObjectOCommand send( const uint32_t cmd );
    void output( std::ostream& ) const; //!< @internal

protected:

    /** @sa co::Object::attachToSession. */
    virtual void attach( const uint128_t& id, const uint32_t instanceID );

    /** @internal Execute the slave remove request. */
    virtual void removeChild( const uint128_t& id );

private:
    /** Number of activations for this pipe. */
    uint32_t _active;

    friend class Node;

    /** The current state for state change synchronization. */
    lunchbox::Monitor< State > _state;

    /** The last draw window for this entity. */
    const Window* _lastDrawWindow;

    struct Private;
    Private* _private; // placeholder for binary-compatible changes

    /* command handler functions. */
    bool _cmdConfigInitReply( co::ICommand& command );
    bool _cmdConfigExitReply( co::ICommand& command );
};
}
}
#endif // EQSERVER_PIPE_H
