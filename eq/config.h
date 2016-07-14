
/* Copyright (c) 2005-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric Stalder@gmail.com>
 *               2011-2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_CONFIG_H
#define EQ_CONFIG_H

#include <eq/api.h>
#include <eq/types.h>

#include <eq/fabric/config.h>        // base class
#include <co/objectHandler.h>        // base class

namespace eq
{
namespace detail { class Config; }

/**
 * A configuration is a visualization session driven by an application.
 *
 * The application Client can choose a configuration from a Server. The Config
 * will be instantiated though the NodeFactory. The Config groups all processes
 * of the application in a single session.
 *
 * A configuration has a number of nodes, which represent the processes involved
 * in it. While the Server drives all nodes, a Config instance in a given
 * process only has its Node instantiated, that is, any given Config has at most
 * one Node.
 *
 * The Config in the application process has access to all Canvas, Segment,
 * Layout, View and Observer instances. Only the active Layout of the each
 * Canvas, the Frustum of each View and the Observer parameters are
 * writable. Views can be sub-classed to attach application-specific data.
 *
 * The render client processes have only access to the current View for each of
 * their channels.
 *
 * @sa fabric::Config for public methods
 */
class Config : public fabric::Config< Server, Config, Observer, Layout, Canvas,
                                      Node, ConfigVisitor >,
               public co::ObjectHandler
{
public:
    typedef fabric::Config< Server, Config, Observer, Layout, Canvas, Node,
                            ConfigVisitor > Super; //!< base class

    /** Construct a new config. @version 1.0 */
    EQ_API explicit Config( ServerPtr parent );

    /** Destruct a config. @version 1.0 */
    EQ_API virtual ~Config();

    /** @name Data Access */
    //@{
    /** @return the local client node. @version 1.0 */
    EQ_API ClientPtr getClient();

    /** @return the local client node. @version 1.0 */
    EQ_API ConstClientPtr getClient() const;

    /**
     * @return the application node.
     * @warning experimental, may not be supported in the future
     */
    EQ_API co::NodePtr getApplicationNode();

    EQ_API co::CommandQueue* getMainThreadQueue(); //!< @internal
    EQ_API co::CommandQueue* getCommandThreadQueue(); //!< @internal

    /** @return the frame number of the last frame started. @version 1.0 */
    EQ_API uint32_t getCurrentFrame() const;

    /** @return the frame number of the last frame finished. @version 1.0 */
    EQ_API uint32_t getFinishedFrame() const;

    /** @internal Get all received statistics. */
    EQ_API GLStats::Data getStatistics() const;

    /**
     * @return true while the config is initialized and no exit event
     *         has happened.
     * @version 1.0
     */
    EQ_API bool isRunning() const;

    /** Stop the config. @version 1.0 */
    EQ_API void stopRunning();

    /**
     * Get the current time in milliseconds.
     *
     * The clock in all processes of the config is synchronized to the Server
     * clock. The precision of this synchronization is typically about 1 ms. The
     * clock of the last instantiated config is used as the lunchbox::Log clock.
     *
     * @return the global time in ms.
     * @version 1.0
     */
    EQ_API int64_t getTime() const;

    /** @return the config's message pump, or 0. @version 1.0 */
    EQ_API MessagePump* getMessagePump();

    /** @internal */
    const Channel* findChannel( const std::string& name ) const
        { return find< Channel >( name ); }
    //@}

    /** @name Operations */
    //@{
    /**
     * Initialize this configuration.
     *
     * This method is to be called only on the application node on an
     * uninitialized configuration.
     *
     * Initializing a configuration starts and connects all render clients,
     * instantiates all active render entities (Node, Pipe, Window, Channel)
     * using the NodeFactory of each process, and calls the configInit task
     * methods on each of these entities, passing the given identifier.
     *
     * The identifier is typically the identifier of a static Object containing
     * initialization data.
     *
     * The initialization fails if at least one of the configInit task methods
     * fails. The application can use sendError() on the render client to pass
     * an error string to the application process, which is received by the
     * normal event processing.
     *
     * @param initID an identifier to be passed to all init methods.
     * @return true if the initialization was successful, false if not.
     * @version 1.0
     */
    EQ_API virtual bool init( const uint128_t& initID );

    /**
     * Exit this configuration.
     *
     * This method is to be called only on the application node on an
     * initialized configuration.
     *
     * Exiting a configuration calls configExit on all active render entities,
     * releases the entities using the NodeFactory and stops the render clients.
     *
     * A configuration which was not exited properly may not be re-initialized.
     *
     * @return true if the exit was successful, false if not.
     * @version 1.0
     */
    EQ_API virtual bool exit();

    /**
     * Update the configuration.
     *
     * This method is to be called only on the application node only on an
     * initialized configuration. Dirty objects on the config are committed,
     * i.e., the View, Canvas and Observer, and any changes to the configuration
     * are effected. Changes may be caused by the eq::admin API or by the
     * application, e.g., through a layout change on a Canvas. Any change causes
     * an implicit finish of all outstanding frames.
     *
     * This function always returns false when a resource failed to initialize
     * or exit. When robustness is not activated, the config is exited by the
     * update upon failure. When robustness is activated, the config keeps
     * running and may be used with reduced functionality.
     *
     * @return true if the configuration update was successful, false if a
     *         resource failed to initialize or exit.
     * @version 1.0
     */
    EQ_API bool update();

    /** @sa fabric::Config::setLatency() */
    EQ_API void setLatency( const uint32_t latency ) override;
    //@}

    /** @name Object registry. */
    //@{
    /**
     * Register a distributed object.
     *
     * Provided for symmetry with deregisterObject. Forwards registration to
     * local client node.
     * @version 1.0
     */
    EQ_API bool registerObject( co::Object* object ) override;

    /**
     * Deregister a distributed object.
     *
     * This method ensures that the data for buffered object is kept for latency
     * frames to allow mapping on slave nodes.
     *
     * @param object the object instance.
     * @version 1.0
     */
    EQ_API void deregisterObject( co::Object* object ) override;

    /**
     * Map a distributed object.
     *
     * Provided for symmetry with deregisterObject. Forwards mapping to
     * local client node.
     * @version 1.0
     */
    EQ_API virtual bool mapObject( co::Object* object, const uint128_t& id,
                                const uint128_t& version = co::VERSION_OLDEST );


    /** Start mapping a distributed object. @version 1.0 */
    EQ_API virtual uint32_t mapObjectNB( co::Object* object, const uint128_t& id,
                                const uint128_t& version = co::VERSION_OLDEST );

    /** Start mapping a distributed object from a known master. @version 1.0 */
    EQ_API uint32_t mapObjectNB( co::Object* object,
                                 const uint128_t& id,
                                 const uint128_t& version,
                                 co::NodePtr master ) override;

    /** Finalize the mapping of a distributed object. @version 1.0 */
    EQ_API bool mapObjectSync( const uint32_t requestID ) override;

    /**
     * Unmap a mapped object.
     *
     * Provided for symmetry with deregisterObject. Forwards unmapping to
     * local client node.
     * @version 1.0
     */
    EQ_API void unmapObject( co::Object* object ) override;

    /**
     * Synchronize the local object with a remote object.
     *
     * Provided for symmetry. Forwards unmapping to local client node.
     * @version 1.7.4
     */
    EQ_API f_bool_t syncObject( co::Object* object, const uint128_t& id,
                                co::NodePtr master,
                                const uint32_t instanceID = CO_INSTANCE_ALL)
        override;
    //@}

    /** @name Frame Control */
    //@{
    /**
     * Request a new frame of rendering.
     *
     * This method is to be called only on the application node on an
     * initialized configuration. It implicitely calls update().
     *
     * The server will sync to the new data, and generate all render tasks,
     * which are queued on the render clients for execution.
     *
     * Each call to startFrame() has to be completed by a finishFrame() or
     * finishAllFrames() before the next call to startFrame().
     *
     * @param frameID a per-frame identifier passed to all rendering methods.
     * @return the frame number of the new frame.
     * @version 1.0
     */
    EQ_API virtual uint32_t startFrame( const uint128_t& frameID );

    /**
     * Finish the rendering of a frame.
     *
     * This method is to be called only on the application node on an
     * initialized configuration.
     *
     * This method synchronizes the local and global rendering. The global
     * rendering is always synchronized to finish the frame (current -
     * latency). The local rendering is synchronized according to the current
     * thread model (cf. Node::IATTR_THREAD_MODEL)
     *
     * @return the frame number of the globally finished frame, or 0 if no
     *         frame has been finished yet.
     * @version 1.0
     */
    EQ_API virtual uint32_t finishFrame();

    /**
     * Finish rendering all pending frames.
     *
     * This method is to be called only on the application node on an
     * initialized configuration.
     *
     * @return the frame number of the last finished frame.
     * @version 1.0
     */
    EQ_API virtual uint32_t finishAllFrames();

    /**
     * Release the local synchronization of the config for a frame.
     *
     * Used by the local Node to release the process-local frame
     * synchronization. An application typically does not call this method
     * directly, it is called from Node::releaseFrameLocal(), which in turn is
     * called from the appropriate task method depending on the thread model.
     *
     * @param frameNumber the frame to release.
     * @version 1.0
     */
    EQ_API void releaseFrameLocal( const uint32_t frameNumber );

    /**
     * Asynchronously signal all channels to interrupt their rendering.
     *
     * This method may be called from any thread in the application process. It
     * causes Channel::notifyStopFrame() to be called immediately on all active
     * channels.
     *
     * @version 1.0
     */
    EQ_API void stopFrames();

    //@}

    /** @name Event handling */
    //@{
#ifndef EQ_2_0_API
    /**
     * Get the next event.
     *
     * To be called only on the application node.
     *
     * The returned event is valid until the next call to this method. This
     * method may block.
     *
     * @return the event.
     * @deprecated
     * @sa Client::processCommand()
     */
    EQ_API const ConfigEvent* nextEvent();

    /**
     * Try to get the next event.
     *
     * To be called only on the application node.
     *
     * The returned event is valid until the next call to this method. This
     * method does not block.
     *
     * @return a config event, or 0 if no events are pending.
     * @deprecated
     */
    EQ_API const ConfigEvent* tryNextEvent();
#endif

    /**
     * Send an (old) event to the application node.
     *
     * Should not be used by applications, other then sending any event defined
     * by Equalizer 1.5.0.
     *
     * @param event the event.
     * @deprecated
     */
    EQ_API void sendEvent( ConfigEvent& event );

    /**
     * Handle one (old) config event. Thread safe.
     *
     * This function handles all events which did exist in Equalizer
     * 1.5.0. Events introduced in 1.5.1 or later are handled by the other
     * handleEvent. Equalizer 2.0 will drop this method and send all events
     * using EventICommand instead of the ConfigEvent struct.
     *
     * @param event the event.
     * @return true if the event requires a redraw, false if not.
     */
    EQ_API virtual bool handleEvent( const ConfigEvent* event );

    /**
     * Send an event to the application node.
     *
     * The returned command can be used to pass additional data to the
     * event. The event will be send after the command is destroyed, aka when it
     * is running out of scope. Thread safe.
     *
     * @param type the event type.
     * @return the event command to pass additional data to
     * @version 1.5.1
     */
    EQ_API EventOCommand sendEvent( const uint32_t type );

    /**
     * Send an error event to the application node.
     *
     * @param type the error event type
     * @param error the error message.
     * @version 1.8.0
     */
    EQ_API EventOCommand sendError( const uint32_t type, const Error& error );

    /** @return the errors since the last call to this method.
     *  @version 1.9
     */
    EQ_API Errors getErrors();

    /**
     * Get the next event.
     *
     * To be called only on the application node.
     *
     * The returned event command is valid until it gets out of scope. This
     * method does not block if the given timeout is 0. Not thread safe.
     *
     * @param timeout time in ms to wait for incoming events
     * @return the event command, or an invalid command on timeout
     * @version 1.5.1
     * @sa Client::processCommand()
     */
    EQ_API EventICommand getNextEvent( const uint32_t timeout =
                                       LB_TIMEOUT_INDEFINITE ) const;

    /**
     * Handle one config event. Thread safe.
     *
     * @param command the event command.
     * @return true if the event requires a redraw, false if not.
     * @version 1.5.1
     */
    EQ_API virtual bool handleEvent( EventICommand command );

    /** @return true if events are pending. Thread safe. @version 1.0 */
    EQ_API bool checkEvent() const;

    /**
     * Handle all config events.
     *
     * To be called only on the application node. Called automatically at the
     * end of each frame to handle pending config events. The default
     * implementation calls handleEvent() on all pending events, without
     * blocking. Not thread safe.
     * @version 1.0
     */
    EQ_API virtual void handleEvents();

    /**
     * Add an statistic event to the statistics overlay. Thread safe.
     *
     * @param originator the originator serial id.
     * @param stat the statistic event.
     * @warning experimental, may not be supported in the future
     */
    void addStatistic( const uint32_t originator, const Statistic& stat );
    //@}

    /**
     * @internal
     * Set up the config's message pump for the given pipe.
     * Used by non-threaded and AGL pipes.
     */
    void setupMessagePump( Pipe* pipe );

    /** @internal Set up appNode connections configured by server. */
    void setupServerConnections( const std::string& connectionData );

protected:
    /** @internal */
    EQ_API void attach( const uint128_t& id,
                        const uint32_t instanceID ) override;

    EQ_API void notifyAttached() override; //!< @internal
    EQ_API void notifyDetach() override; //!< @internal
    /** @internal */
    EQ_API void changeLatency( const uint32_t latency ) override;
    EQ_API bool mapViewObjects() const override; //!< @internal

private:
    detail::Config* const _impl;

    void _frameStart();
    friend class Node;

    bool _needsLocalSync() const;

    bool _handleNewEvent( EventICommand& command );
    bool _handleEvent( const Event& event );
    const ConfigEvent* _convertEvent( co::ObjectICommand command );

    /** Update statistics for the last finished frame */
    void _updateStatistics();

    /** Release all deregistered buffered objects after their latency is
        done. */
    void _releaseObjects();

    /** Exit the current message pump */
    void _exitMessagePump();

    /** The command functions. */
    bool _cmdSyncClock( co::ICommand& command );
    bool _cmdCreateNode( co::ICommand& command );
    bool _cmdDestroyNode( co::ICommand& command );
    bool _cmdInitReply( co::ICommand& command );
    bool _cmdExitReply( co::ICommand& command );
    bool _cmdUpdateVersion( co::ICommand& command );
    bool _cmdUpdateReply( co::ICommand& command );
    bool _cmdReleaseFrameLocal( co::ICommand& command );
    bool _cmdFrameFinish( co::ICommand& command );
    bool _cmdSwapObject( co::ICommand& command );
};
}

#endif // EQ_CONFIG_H
