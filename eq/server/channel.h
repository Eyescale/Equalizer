
/* Copyright (c) 2005-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQSERVER_CHANNEL_H
#define EQSERVER_CHANNEL_H

#include <eq/server/api.h>
#include "state.h"  // enum
#include "types.h"

#include <eq/fabric/channel.h>       // base class
#include <eq/fabric/pixelViewport.h> // member
#include <eq/fabric/viewport.h>      // member
#include <lunchbox/monitor.h> // member

#include <iostream>
#include <vector>

namespace eq
{
namespace server
{
class Channel : public fabric::Channel< Window, Channel >
{
public:
    /** Construct a new channel. */
    EQSERVER_API explicit Channel( Window* parent );

    /** Construct a copy of a channel. */
    Channel( const Channel& from );

    /** Destruct this channel. */
    virtual ~Channel();

    /** @return the state of this channel. */
    State getState() const { return _state.get(); }

    /** @internal */
    void setState( const State state ) { _state = state; }

    /**
     * @name Data Access
     */
    //@{
    Config* getConfig();
    const Config* getConfig() const;

    EQSERVER_API Node* getNode();
    EQSERVER_API const Node* getNode() const;

    Pipe* getPipe();
    const Pipe* getPipe() const;

    /** @return the parent server. @version 1.0 */
    ServerPtr getServer();

    const Compounds& getCompounds() const;

    co::CommandQueue* getMainThreadQueue();
    co::CommandQueue* getCommandThreadQueue();

    /** Increase channel activition count. */
    void activate();

    /** Decrease channel activition count. */
    void deactivate();

    /** @return if this channel is actively used for rendering. */
    bool isActive() const { return (_active != 0); }

    /** @return if this window is running. */
    bool isRunning() const { return _state == STATE_RUNNING; }

    /** Schedule deletion of this channel. */
    void postDelete();

    /**
     * Add additional tasks this channel, and all its parents, might
     * potentially execute.
     */
    void addTasks( const uint32_t tasks );

    /** Set the output view and segment for this channel. */
    void setOutput( View* view, Segment* segment );

    /** Unset the output view and segment for this channel. */
    void unsetOutput();

    /** @return the channel's view. */
    const View* getView() const { return _view; }

    /** @return the channel's canvas. */
    const Canvas* getCanvas() const;

    /** @return the channel's view. */
    View* getView() { return _view; }

    /**
     * @return true if this channel supports the capabilities needed for
     *         the view.
     */
    bool supportsView( const View* view ) const;

    /** @return the channel's layout. */
    EQSERVER_API const Layout* getLayout() const;

    /** @return the channel's segment. */
    const Segment* getSegment() const { return _segment; }

    /** @return the channel's segment. */
    Segment* getSegment() { return _segment; }

    /** The last drawing compound for this entity. */
    void setLastDrawCompound( const Compound* compound )
        { _lastDrawCompound = compound; }
    const Compound* getLastDrawCompound() const { return _lastDrawCompound;}

    void setIAttribute( const IAttribute attr, const int32_t value )
        { fabric::Channel< Window, Channel >::setIAttribute( attr, value );}
    void setSAttribute( const SAttribute attr, const std::string& value )
        { fabric::Channel< Window, Channel >::setSAttribute( attr, value );}
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
     * Update one frame.
     *
     * @param frameID a per-frame identifier passed to all rendering
     *                methods.
     * @param frameNumber the number of the frame.
     * @return true if at least one rendering task was sent.
     */
    bool update( const uint128_t& frameID, const uint32_t frameNumber );

    co::ObjectOCommand send( const uint32_t cmd );
    //@}

    /** @name Channel listener interface. */
    //@{
    /** Register a channel listener. */
    void addListener( ChannelListener* listener );
    /** Deregister a channel listener. */
    void removeListener( ChannelListener* listener );
    /** @return true if the channel has listeners */
    bool hasListeners() const { return !_listeners.empty(); }
    //@}

    bool omitOutput() const; //!< @internal
    void output( std::ostream& ) const; //!< @internal

protected:
    /** @sa net::Object::attach. */
    virtual void attach( const uint128_t& id, const uint32_t instanceID );

private:
    //-------------------- Members --------------------
    /** Number of activations for this channel. */
    uint32_t _active;

    /** The view used by this channel. */
    View* _view;

    /** The segment used by this channel. */
    Segment* _segment;

    Vector4i _overdraw;

    /** The current state for state change synchronization. */
    lunchbox::Monitor< State > _state;

    /** The last draw compound for this entity */
    const Compound* _lastDrawCompound;

    typedef std::vector< ChannelListener* > ChannelListeners;
    ChannelListeners _listeners;

    LB_TS_VAR( _serverThread );

    struct Private;
    Private* _private; // placeholder for binary-compatible changes

    //-------------------- Methods --------------------
    Vector3ub _getUniqueColor() const;

    void _setupRenderContext( const uint128_t& frameID,
                              RenderContext& context );

    void _fireLoadData( const uint32_t frameNumber,
                        const Statistics& statistics,
                        const Viewport& region );

    /* command handler functions. */
    bool _cmdConfigInitReply( co::ICommand& command );
    bool _cmdConfigExitReply( co::ICommand& command );
    bool _cmdFrameFinishReply( co::ICommand& command );
    bool _cmdNop( co::ICommand& /*command*/ )
        { return true; }

    virtual void updateCapabilities();
};
}
}
#endif // EQSERVER_CHANNEL_H
