
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric Stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQSERVER_NODE_H
#define EQSERVER_NODE_H

#include "config.h"                // used in inline method
#include "state.h"                 // enum
#include "types.h"

#include <eq/fabric/node.h> // base class

#include <co/barrier.h>
#include <co/bufferConnection.h>
#include <co/connectionDescription.h>
#include <co/node.h>

#include <vector>

namespace eq
{
namespace server
{
/** The node. */
class Node : public fabric::Node< Config, Node, Pipe, NodeVisitor >
{
public:
    /** Construct a new Node. */
    EQSERVER_API explicit Node( Config* parent );

    virtual ~Node();

    /** @name Data Access. */
    //@{
    ServerPtr getServer();
    ConstServerPtr getServer() const;

    co::NodePtr getNode() const { return _node; }
    void setNode( co::NodePtr node ) { _node = node; }

    void setHost( const std::string& host ) { _host = host; }
    const std::string& getHost() const { return _host; }

    Channel* getChannel( const ChannelPath& path );

    /** @return the state of this node. */
    State getState()    const { return _state.get(); }

    /** @internal */
    void setState( const State state ) { _state = state; }

    co::CommandQueue* getMainThreadQueue();
    co::CommandQueue* getCommandThreadQueue();

    /** Increase node activition count. */
    void activate();

    /** Decrease node activition count. */
    void deactivate();

    /** @return if this pipe is actively used for rendering. */
    bool isActive() const { return ( _active != 0 ); }

    /** @return if this node is running. */
    bool isRunning() const { return _state == STATE_RUNNING; }

    /** @return if this node is stopped. */
    bool isStopped() const { return _state == STATE_STOPPED; }

    /**
     * Add additional tasks this pipe, and all its parents, might
     * potentially execute.
     */
    void addTasks( const uint32_t tasks );

    /** The last drawing channel for this entity. */
    void setLastDrawPipe( const Pipe* pipe )
    { _lastDrawPipe = pipe; }
    const Pipe* getLastDrawPipe() const { return _lastDrawPipe;}

    /** @return the number of the last finished frame. @internal */
    uint32_t getFinishedFrame() const { return _finishedFrame; }
    //@}

    /**
     * @name Operations
     */
    //@{
    /** Connect the render slave node process. */
    bool connect();

    /** Launch the render slave node process. */
    bool launch();

    /** Synchronize the connection of a render slave launch. */
    bool syncLaunch( const lunchbox::Clock& time );

    /** Start initializing this entity. */
    void configInit( const uint128_t& initID, const uint32_t frameNumber );

    /** Sync initialization of this entity. */
    bool syncConfigInit();

    /** Start exiting this entity. */
    void configExit();

    /** Sync exit of this entity. */
    bool syncConfigExit();

    /**
     * Trigger the rendering of a new frame for this node.
     *
     * @param frameID a per-frame identifier passed to all rendering
     *                methods.
     * @param frameNumber the number of the frame.
     */
    void update( const uint128_t& frameID, const uint32_t frameNumber );

    /**
     * Flush the processing of frames, including frameNumber.
     *
     * @param frameNumber the number of the frame.
     */
    void flushFrames( const uint32_t frameNumber );

    /** Synchronize the completion of the rendering of a frame. */
    void finishFrame( const uint32_t frame );
    //@}

    /**
     * @name Barrier Cache
     *
     * Caches barriers for which this node is the master.
     */
    //@{
    /**
     * Get a new barrier of height 0.
     *
     * @return the barrier.
     */
    co::Barrier* getBarrier();

    /**
     * Release a barrier server by this node.
     *
     * @param barrier the barrier.
     */
    void releaseBarrier( co::Barrier* barrier );

    /** Change the latency on all objects (barrier) */
    void changeLatency( const uint32_t latency );
    //@}

    co::ObjectOCommand send( const uint32_t cmd );
    co::ObjectOCommand send( const uint32_t cmd, const uint128_t& id );
    EventOCommand sendError( const uint32_t error );

    void flushSendBuffer();

    /**
     * Add a new description how this node can be reached.
     *
     * @param desc the connection description.
     */
    void addConnectionDescription( co::ConnectionDescriptionPtr desc )
    { _connectionDescriptions.push_back( desc ); }

    /**
     * Remove a connection description.
     *
     * @param cd the connection description.
     * @return true if the connection description was removed, false otherwise.
     */
    EQSERVER_API bool removeConnectionDescription(
        co::ConnectionDescriptionPtr cd );

    /** @return the vector of connection descriptions. */
    const co::ConnectionDescriptions& getConnectionDescriptions()
        const { return _connectionDescriptions; }


    /** @name Attributes */
    //@{
    /** String attributes. */
    enum SAttribute
    {
        SATTR_LAUNCH_COMMAND, //!< the command to launch the node
        SATTR_LAST,
        SATTR_ALL = SATTR_LAST + 5
    };

    /** Character attributes. */
    enum CAttribute
    {
        CATTR_LAUNCH_COMMAND_QUOTE, //!< The character to quote arguments
        CATTR_LAST,
        CATTR_ALL = CATTR_LAST + 5
    };

    /** @internal Set a string integer attribute. */
    EQSERVER_API void setSAttribute( const SAttribute attr, const std::string& value );

    /** @internal Set a character integer attribute. */
    void setCAttribute( const CAttribute attr, const char value );

    /** @return the value of a node string attribute. @version 1.0 */
    const std::string& getSAttribute( const SAttribute attr ) const;

    /** @return the value of a node string attribute. @version 1.0 */
    char getCAttribute( const CAttribute attr ) const;

    /** @internal @return the name of a node string attribute. */
    static const std::string& getSAttributeString( const SAttribute attr );
    /** @internal @return the name of a node character attribute. */
    static const std::string& getCAttributeString( const CAttribute attr );
    //@}

    void output( std::ostream& os ) const; //!< @internal

protected:

    /** @sa co::Object::attach. */
    virtual void attach( const uint128_t& id, const uint32_t instanceID );

private:
    /** String attributes. */
    std::string _sAttributes[SATTR_ALL];

    /** Character attributes. */
    char _cAttributes[CATTR_ALL];

    std::string _host; // The host name to launch this node

    /** Number of activations for this node. */
    uint32_t _active;

    /** The network node on which this Equalizer node is running. */
    co::NodePtr _node;

    /** The list of descriptions on how this node is reachable. */
    co::ConnectionDescriptions _connectionDescriptions;

    typedef stde::hash_map< uint32_t, co::uint128_t > FrameIDHash;
    /** The frame identifiers non-finished frames. */
    FrameIDHash _frameIDs;

    /** The number of the last finished frame. */
    uint32_t _finishedFrame;

    /** The number of the last flushed frame (frame finish command sent). */
    uint32_t _flushedFrame;

    /** The current state for state change synchronization. */
    lunchbox::Monitor< State > _state;

    /** The cached barriers. */
    std::vector<co::Barrier*> _barriers;

    /** Task commands for the current operation. */
    co::BufferConnectionPtr _bufferedTasks;

    /** The last draw pipe for this entity */
    const Pipe* _lastDrawPipe;

    struct Private;
    Private* _private; // placeholder for binary-compatible changes

    /**
     * Compose and execute the launch command by expanding the variables in
     * the launch command string.
     *
     * @param description the connection description.
     * @return true on success, false otherwise
     */
    bool _launch( const std::string& hostname ) const;
    std::string _createLaunchCommand() const;
    std::string   _createRemoteCommand() const;

    uint32_t _getFinishLatency() const;
    void _finish( const uint32_t currentFrame );

    /** flush cached barriers. */
    void _flushBarriers();

    /** Send the frame finish command for the given frame number. */
    void _sendFrameFinish( const uint32_t frameNumber );

    /* ICommand handler functions. */
    bool _cmdConfigInitReply( co::ICommand& command );
    bool _cmdConfigExitReply( co::ICommand& command );
    bool _cmdFrameFinishReply( co::ICommand& command );
};
}
}
#endif // EQSERVER_NODE_H
