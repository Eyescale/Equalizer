
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric Stalder@gmail.com> 
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

#include <eq/commandQueue.h>  // member
#include <eq/types.h>         // typedefs
#include <eq/visitorResult.h> // enum

#include <eq/fabric/config.h>        // base class
#include <co/base/monitor.h>         // member
#include <co/base/spinLock.h>        // member

namespace eq
{
    /**
     * A configuration is a visualization session driven by an application.
     *
     * The application Client can choose a configuration from a Server. The
     * Config will be instantiated though the NodeFactory. The Config groups all
     * processes of the application in a single session.
     *
     * A configuration has a number of nodes, which represent the processes
     * involved in it. While the Server drives all nodes, a Config instance in a
     * given process only has its Node instantiated, that is, any given Config
     * has at most one Node.
     *
     * The Config in the application process has access to all Canvas, Segment,
     * Layout, View and Observer instances. Only the active Layout of the each
     * Canvas, the Frustum of each View and the Observer parameters are
     * writable. Views can be subclassed to attach application-specific data.
     *
     * The render client processes have only access to the current View for each
     * of their channels.
     *
     * @sa fabric::Config for public methods
     */
    class Config : public fabric::Config< Server, Config, Observer, Layout,
                                          Canvas, Node, ConfigVisitor >
    {
    public:
        typedef fabric::Config< Server, Config, Observer, Layout, Canvas, Node,
                                ConfigVisitor > Super; //!< base class

        /** Construct a new config. @version 1.0 */
        EQ_API Config( ServerPtr parent );

        /** Destruct a config. @version 1.0 */
        EQ_API virtual ~Config();

        /** @name Data Access */
        //@{
        /** @return the local client node. @version 1.0 */
        EQ_API ClientPtr getClient();

        /** @return the local client node. @version 1.0 */
        EQ_API ConstClientPtr getClient() const;

        EQ_API co::CommandQueue* getMainThreadQueue(); //!< @internal
        EQ_API co::CommandQueue* getCommandThreadQueue(); //!< @internal

        /** @return the frame number of the last frame started. @version 1.0 */
        uint32_t getCurrentFrame()  const { return _currentFrame; }

        /** @return the frame number of the last frame finished. @version 1.0 */
        uint32_t getFinishedFrame() const { return _finishedFrame.get(); }

        /** @internal Get all received statistics. */
        EQ_API void getStatistics( std::vector< FrameStatistics >& stats );

        /**
         * @return true while the config is initialized and no exit event
         *         has happened.
         * @version 1.0
         */
        bool isRunning() const { return _running; }

        /** Stop the config. @version 1.0 */
        void stopRunning() { _running = false; }

        /**
         * Get the current time in milliseconds.
         *
         * The clock in all processes of the config is synchronized to the
         * Server clock. The precision of this synchronization is typically
         * about 1 ms. The clock of the last instantiated config is used as the
         *co::base::Log clock.
         *
         * @return the global time in ms.
         * @version 1.0
         */
        int64_t getTime() const { return _clock.getTime64(); }

        /** @return the config's message pump, or 0. @version 1.0 */
        MessagePump* getMessagePump();

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
         * using the NodeFactory of each process, and calls the configInit
         * task methods on each of these entities, passing the given identifier.
         *
         * The identifier is typically the identifier of a static Object
         * containing initialization data.
         *
         * The initialization fails if at least one of the configInit task
         * methods fails. The application can use setErrorMessage on the render
         * client to pass an error string to the application process, which it
         * can query it using getError().
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
         * Exiting a configuration calls configExit on all active render
         * entities, releases the entities using the NodeFactory and stops the
         * render clients.
         *
         * A configuration which was not exited properly may not be
         * re-initialized.
         *
         * @return true if the exit was successful, false if not.
         * @version 1.0
         */
        EQ_API virtual bool exit();

        /** 
         * Update the configuration.
         *
         * This method is to be called only on the application node on an
         * initialized configuration. Dirty objects on the config are committed,
         * i.e., the View, Canvas and Observer, and any changes on the
         * configuration are effected. Changes may be caused by the eq::admin
         * API or by the application, e.g., through a layout change on a
         * Canvas. Any change causes an implicit finish of all outstanding
         * frames.
         *
         * This function always returns false when a resource failed to
         * initialize or exit. When robustness is not activated, the config is
         * exited by the update upon failure. When robustness is activated, the
         * config keeps running and may be used with reduced functionality.
         *
         * @return true if the configuration update was successful, false if a
         *         resource failed to initialize or exit.
         * @version 1.0
         */
        EQ_API bool update();
        
        /** @warning Experimental - may not be supported in the future */
        EQ_API void freezeLoadBalancing( const bool onOff );

        /** @sa fabric::Config::setLatency() */
        EQ_API virtual void setLatency( const uint32_t latency );
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
        EQ_API virtual bool registerObject( co::Object* object );

        /**
         * Deregister a distributed object.
         *
         * This method ensures that the data for buffered object is kept for
         * latency frames to allow mapping on slave nodes.
         *
         * @param object the object instance.
         * @version 1.0
         */
        EQ_API virtual void deregisterObject( co::Object* object );

        /** 
         * Map a distributed object.
         *
         * Provided for symmetry with deregisterObject. Forwards mapping to
         * local client node.
         * @version 1.0
         */
        EQ_API virtual bool mapObject( co::Object* object,
                                       const co::base::UUID& id, 
                               const uint128_t& version = co::VERSION_OLDEST );


        /** Start mapping a distributed object. @version 1.0 */
        EQ_API virtual uint32_t mapObjectNB( co::Object* object,
                                             const co::base::UUID& id, 
                                const uint128_t& version = co::VERSION_OLDEST );
        /** Finalize the mapping of a distributed object. @version 1.0 */
        EQ_API virtual bool mapObjectSync( const uint32_t requestID );

        /** 
         * Unmap a mapped object.
         * 
         * Provided for symmetry with deregisterObject. Forwards unmapping to
         * local client node.
         * @version 1.0
         */
        EQ_API virtual void unmapObject( co::Object* object );

        /** Convenience method to deregister or unmap an object. @version 1.0 */
        EQ_API void releaseObject( co::Object* object );
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
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
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
         * latency). The local rendering is synchronized according to the
         * current thread model (cf. Node::IATTR_THREAD_MODEL)
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
         * directly, it is called from Node::releaseFrameLocal(), which in turn
         * is called from the appropriate task method depending on the thread
         * model.
         *
         * @param frameNumber the frame to release.
         * @version 1.0
         */
        void releaseFrameLocal( const uint32_t frameNumber )
            { _unlockedFrame = frameNumber; }

        /**
         * Asynchronously signal all channels to interrupt their rendering.
         *
         * This method may be called from any thread in the application
         * process. It causes Channel::notifyStopFrame() to be called
         * immediately on all active channels.
         *
         * @version 1.0
         */
        EQ_API void stopFrames();

        //@}

        /** @name Event handling */
        //@{
        /** 
         * Send an event to the application node.
         *
         * @param event the event.
         * @version 1.0
         */
        EQ_API void sendEvent( ConfigEvent& event );

        /** 
         * Get the next event.
         * 
         * To be called only on the application node.
         * 
         * The returned event is valid until the next call to this method. This
         * method may block.
         * 
         * @return the event.
         * @version 1.0
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
         * @version 1.0
         */
        EQ_API const ConfigEvent* tryNextEvent();

        /** @return true if events are pending. @version 1.0 */
        bool checkEvent() const { return !_eventQueue.isEmpty(); }

        /**
         * Handle all config events.
         *
         * To be called only on the application node.
         * 
         * Called automatically at the end of each frame to handle pending
         * config events. The default implementation calls handleEvent() on all
         * pending events, without blocking.
         * @version 1.0
         */
        EQ_API virtual void handleEvents();

        /** 
         * Handle one config event.
         * 
         * @param event the event.
         * @return true if the event requires a redraw, false if not.
         * @version 1.0
         */
        EQ_API virtual bool handleEvent( const ConfigEvent* event );
        //@}
        
        /** 
         * @internal
         * Set up the config's message pump for the given pipe.
         * Used by non-threaded and AGL pipes.
         */
        void setupMessagePump( Pipe* pipe );

        /** @internal Set up appNode connections configured by server. */
        void setupServerConnections( const char* connectionData );

    protected:
        /** @internal */
        EQ_API virtual void attach( const co::base::UUID& id,
                                    const uint32_t instanceID );

        EQ_API virtual void notifyAttached(); //!< @internal
        EQ_API virtual void notifyDetach(); //!< @internal
        /** @internal */
        EQ_API virtual void changeLatency( const uint32_t latency );
        EQ_API virtual bool mapViewObjects() const; //!< @internal

    private:
        /** The node running the application thread. */
        co::NodePtr _appNode;

        /** The receiver->app thread event queue. */
        CommandQueue _eventQueue;
        
        /** The last received event to be released. */
        co::Command* _lastEvent;

        /** The connections configured by the server for this config. */
        co::Connections _connections;

        /** Global statistics events, index per frame and channel. */
        std::deque< FrameStatistics > _statistics;
       co::base::Lock                    _statisticsMutex;
        
        /** The last started frame. */
        uint32_t _currentFrame;
        /** The last locally released frame. */
        uint32_t _unlockedFrame;
        /** The last completed frame. */
       co::base::Monitor< uint32_t > _finishedFrame;

        /** The global clock. */
       co::base::Clock _clock;

        std::deque< int64_t > _frameTimes; //!< Start time of last frames

        /** true while the config is initialized and no window has exited. */
        bool _running;

        /** @internal A proxy object to keep master data within latency. */
        class LatencyObject : public co::Object
        {
        public:
            LatencyObject( const ChangeType type ) : _changeType( type ) {}
            uint32_t frameNumber;

        protected:
            virtual ChangeType getChangeType() const { return _changeType; }
            virtual void getInstanceData( co::DataOStream& os ){ EQDONTCALL }
            virtual void applyInstanceData( co::DataIStream& is ){ EQDONTCALL }
        private:
            const ChangeType _changeType;
        };
        
        /** list of the current latency object */
        typedef std::vector< LatencyObject* > LatencyObjects;

        /** protected list of the current latency object */
       co::base::Lockable< LatencyObjects,co::base::SpinLock > _latencyObjects;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        friend class Node;
        void _frameStart();
        bool _needsLocalSync() const;

        /** 
         * Update statistics for the finished frame, push it to the local node
         * for visualization.
         */
        void _updateStatistics( const uint32_t finishedFrame );

        /** Release all deregistered buffered objects after their latency is
            done. */
        void _releaseObjects();

        /** Exit the current message pump */
        void _exitMessagePump();

        /** The command functions. */
        bool _cmdSyncClock( co::Command& command );
        bool _cmdCreateNode( co::Command& command );
        bool _cmdDestroyNode( co::Command& command );
        bool _cmdInitReply( co::Command& command );
        bool _cmdExitReply( co::Command& command );
        bool _cmdUpdateVersion( co::Command& command );
        bool _cmdUpdateReply( co::Command& command );
        bool _cmdReleaseFrameLocal( co::Command& command );
        bool _cmdFrameFinish( co::Command& command );
        bool _cmdSwapObject( co::Command& command );
    };
}

#endif // EQ_CONFIG_H

