
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder<cedric.stalder@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation .
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

#ifndef EQ_NODE_H
#define EQ_NODE_H

#include <eq/api.h>
#include <eq/types.h>
#include <eq/visitorResult.h>  // enum
#include <eq/fabric/node.h> // base class

#include <co/types.h>

namespace eq
{
namespace detail { class Node; }

/**
 * A Node represents a single computer in the cluster.
 *
 * Each node is executed in a separate process. Each process has only its local
 * node instantiated, that is, it has at most instance of a Node and does not
 * see other node instances. The application process may not have a node, which
 * is the case when it does not contribute to the rendering.
 *
 * The eq::Node is not to be confused with the co::Node which represents the
 * process in the underlying peer-to-peer network layer. The eq::Client and
 * eq::Server are co::Nodes representing the local client and Equalizer server,
 * respectively.
 *
 * @sa fabric::Node
 */
class Node : public fabric::Node< Config, Node, Pipe, NodeVisitor >
{
public:
    /** Construct a new node. @version 1.0 */
    EQ_API explicit Node( Config* parent );

    /** Destruct the node. @version 1.0 */
    EQ_API virtual ~Node();

    /** @return the parent client node. @version 1.0 */
    EQ_API ClientPtr getClient();

    /** @return the parent server node. @version 1.0 */
    EQ_API ServerPtr getServer();

    EQ_API co::CommandQueue* getMainThreadQueue(); //!< @internal
    EQ_API co::CommandQueue* getCommandThreadQueue(); //!< @internal
    co::CommandQueue* getTransmitterQueue(); //!< @internal

    /** @internal node thread only. */
    uint32_t getCurrentFrame() const;

    /**
     * @internal
     * Get a network barrier.
     *
     * @param barrier the barrier identifier and version.
     * @return the barrier.
     */
    co::Barrier* getBarrier( const co::ObjectVersion& barrier );

    /**
     * @internal
     * Get a frame data instance.
     *
     * @param frameDataVersion the frame data identifier and version.
     * @return the frame.
     */
    FrameDataPtr getFrameData( const co::ObjectVersion& frameDataVersion );

    /** @internal Release the frame data instance. */
    void releaseFrameData( FrameDataPtr data );

    /** @internal Wait for the node to be initialized. */
    EQ_API void waitInitialized() const;

    /**
     * @return true if this node is running, false otherwise.
     * @version 1.0
     */
    EQ_API bool isRunning() const;

    /**
     * @return true if this node is stopped, false otherwise.
     * @version 1.0
     */
    EQ_API bool isStopped() const;

    /**
     * Wait for a frame to be started.
     *
     * Used by the pipe task methods to implement the current thread
     * synchronization model.
     *
     * @param frameNumber the frame number.
     * @sa releaseFrame()
     * @version 1.0
     */
    EQ_API void waitFrameStarted( const uint32_t frameNumber ) const;

    /** @internal @return the number of the last finished frame. */
    uint32_t getFinishedFrame() const;

    /**
     * Send an error event to the application node.
     *
     * @param error the error code.
     * @version 1.7.1
     */
    EQ_API EventOCommand sendError( const uint32_t error );

    /**
     * Process a received event.
     *
     * The task of this method is to update the node as necessary, and transform
     * the event into a config event to be send to the application using
     * Config::sendEvent().
     *
     * @param event the received event.
     * @return true when the event was handled, false if not.
     * @version 1.5.2
     */
    EQ_API virtual bool processEvent( const Event& event );

    /** @internal @sa Serializable::setDirty() */
    EQ_API virtual void setDirty( const uint64_t bits );

    /** @internal */
    EQ_API void dirtyClientExit();

protected:
    /** @internal */
    EQ_API virtual void attach( const uint128_t& id, const uint32_t instanceID );

    /** @name Actions */
    //@{
    /**
     * Start a frame by unlocking all child resources.
     *
     * @param frameNumber the frame to start.
     * @version 1.0
     */
    EQ_API void startFrame( const uint32_t frameNumber );

    /**
     * Signal the completion of a frame to the parent.
     *
     * @param frameNumber the frame to end.
     * @version 1.0
     */
    EQ_API void releaseFrame( const uint32_t frameNumber );

    /**
     * Release the local synchronization of the parent for a frame.
     *
     * @param frameNumber the frame to release.
     * @version 1.0
     */
    EQ_API void releaseFrameLocal( const uint32_t frameNumber );
    //@}

    /**
     * @name Callbacks
     *
     * The callbacks are called by Equalizer during rendering to execute
     * various actions.
     */
    //@{

    /**
     * Initialize this node.
     *
     * @param initID the init identifier.
     * @version 1.0
     */
    EQ_API virtual bool configInit( const uint128_t&  initID );

    /** Exit this node. @version 1.0 */
    EQ_API virtual bool configExit();

    /**
     * Start rendering a frame.
     *
     * Called once at the beginning of each frame, to start the node's frame
     * and to do per-frame updates of node-specific data. This method has to
     * call startFrame(). Immediately releases local synchronization if the
     * thread model is async.
     *
     * @param frameID the per-frame identifier.
     * @param frameNumber the frame to start.
     * @sa startFrame(), Config::beginFrame()
     * @version 1.0
     */
    EQ_API virtual void frameStart( const uint128_t& frameID,
                                    const uint32_t frameNumber );

    /**
     * Finish rendering a frame.
     *
     * Called once at the end of each frame, to end the frame and to do
     * per-frame updates of node-specific data. This method has to call
     * releaseFrame().
     *
     * @param frameID the per-frame identifier.
     * @param frameNumber the frame to finish.
     * @sa endFrame(), Config::finishFrame()
     * @version 1.0
     */
    EQ_API virtual void frameFinish( const uint128_t& frameID,
                                     const uint32_t frameNumber );

    /**
     * Finish drawing.
     *
     * Called once per frame after the last draw operation. Waits for the
     * pipes to release the local synchonization and releases the node's
     * local synchronization if the thread model is draw_sync (the default).
     *
     * @param frameID the per-frame identifier.
     * @param frameNumber the frame finished with draw.
     * @sa Pipe::waitFrameLocal(), releaseFrameLocal()
     * @version 1.0
     */
    EQ_API virtual void frameDrawFinish( const uint128_t& frameID,
                                         const uint32_t frameNumber );

    /**
     * Finish all rendering tasks.
     *
     * Called once per frame after all frame tasks.  Waits for the pipes to
     * release the local synchonization and releases the node's local
     * synchronization if the thread model is local_sync.
     *
     * Note that frameFinish is called after the latency is exhausted and
     * synchronizes pipe thread execution.
     *
     * @param frameID the per-frame identifier.
     * @param frameNumber the frame finished with draw.
     * @sa Pipe::waitFrameLocal(), releaseFrameLocal()
     * @version 1.0
     */
    EQ_API virtual void frameTasksFinish( const uint128_t& frameID,
                                          const uint32_t frameNumber );
    //@}

private:
    detail::Node* const _impl;

    void _setAffinity();

    void _finishFrame( const uint32_t frameNumber ) const;
    void _frameFinish( const uint128_t& frameID,
                       const uint32_t frameNumber );

    void _flushObjects();

    /** The command functions. */
    bool _cmdCreatePipe( co::ICommand& command );
    bool _cmdDestroyPipe( co::ICommand& command );
    bool _cmdConfigInit( co::ICommand& command );
    bool _cmdConfigExit( co::ICommand& command );
    bool _cmdFrameStart( co::ICommand& command );
    bool _cmdFrameFinish( co::ICommand& command );
    bool _cmdFrameDrawFinish( co::ICommand& command );
    bool _cmdFrameTasksFinish( co::ICommand& command );
    bool _cmdFrameDataTransmit( co::ICommand& command );
    bool _cmdFrameDataReady( co::ICommand& command );
    bool _cmdSetAffinity( co::ICommand& command );

    LB_TS_VAR( _nodeThread );
};
}

#endif // EQ_NODE_H
