
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

#include <eq/client/commandQueue.h>  // member
#include <eq/client/observer.h>      // member
#include <eq/client/types.h>         // typedefs
#include <eq/client/visitorResult.h> // enum

#include <eq/net/session.h>          // base class
#include <eq/base/monitor.h>         // member

namespace eq
{
    class ConfigDeserializer;
    class ConfigVisitor;
    class Node;
    class SceneObject;
    struct ConfigEvent;

    /**
     * A configuration is a visualization session driven by an application.
     *
     * The application Client can choose a configuration from a Server. The
     * Config will be instantiated though the NodeFactory. The Config groups all
     * processes of the application in a single net::Session.
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
     */
    class Config : public net::Session
    {
    public:
        /** Construct a new config. @version 1.0 */
        EQ_EXPORT Config( ServerPtr parent );

        /** Destruct a config. @version 1.0 */
        EQ_EXPORT virtual ~Config();

        /** @name Data Access */
        //@{
        /** @return the name of this config. @version 1.0 */
        const std::string& getName() const { return _name; }

        /** @return the local client node. @version 1.0 */
        EQ_EXPORT ClientPtr getClient();
        
        /** @return the local server proxy. @version 1.0 */
        EQ_EXPORT ServerPtr getServer();

        /** @internal */
        EQ_EXPORT CommandQueue* getNodeThreadQueue();

        /** @return the frame number of the last frame started. @version 1.0 */
        uint32_t getCurrentFrame()  const { return _currentFrame; }

        /** @return the frame number of the last frame finished. @version 1.0 */
        uint32_t getFinishedFrame() const { return _finishedFrame.get(); }

        /**
         * @return the latency, i.e., the maximum distance between the finished
         *          and started frame.
         * @version 1.0
         */
        uint32_t getLatency() const { return _latency; }

        /**
         * Change the latency of the configuration.
         *
         * The config has to be running. Pending rendering frames are finished
         * by this method, making it relatively expensive.
         *
         * @warning Experimental - may not be supported in the future
         */
        EQ_EXPORT void changeLatency( const uint32_t latency );

        /**
         * @return the vector of nodes instantiated on this process.
         * @version 1.0
         */
        const NodeVector& getNodes() const { return _nodes; }

        /** @return the vector of observers, app-node only. @version 1.0 */
        const ObserverVector& getObservers() const { return _observers; }

        /** @return the observer of the given identifier, or 0. @version 1.0 */
        Observer* findObserver( const uint32_t id );

        /** @return the observer of the given identifier, or 0. @version 1.0 */
        const Observer* findObserver( const uint32_t id ) const;

        /** @return the vector of layouts, app-node only. @version 1.0 */
        const LayoutVector& getLayouts() const { return _layouts; }

        /** @return the layout of the given identifier, or 0. @version 1.0 */
        EQ_EXPORT Layout* findLayout( const uint32_t id );

        /** @return the view of the given identifier, or 0. @version 1.0 */
        EQ_EXPORT View* findView( const uint32_t id );

        /** @return the vector of canvases, app-node only. @version 1.0 */
        const CanvasVector& getCanvases() const { return _canvases; }

        /** 
         * Traverse this config and all children using a config visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQ_EXPORT VisitorResult accept( ConfigVisitor& visitor );

        /** Const-version of accept(). @version 1.0 */
        EQ_EXPORT VisitorResult accept( ConfigVisitor& visitor ) const;

        /** Get all received statistics. @internal */
        EQ_EXPORT void getStatistics( std::vector< FrameStatistics >& stats );

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
         * The clock in all processes of the Config is synchronized to the
         * Server clock. The precision of this synchronization is typically
         * about 1 ms. The clock of the last instantiated Config is used as the
         * Log clock.
         *
         * @return the global time in ms.
         * @version 1.0
         */
        int64_t getTime() const { return _clock.getTime64(); }
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
         * can query it using getErrorMessage().
         * 
         * @param initID an identifier to be passed to all init methods.
         * @return true if the initialization was successful, false if not.
         * @version 1.0
         */
        EQ_EXPORT virtual bool init( const uint32_t initID );

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
        EQ_EXPORT virtual bool exit();

        /** @warning Experimental - may not be supported in the future */
        EQ_EXPORT void freezeLoadBalancing( const bool onOff );
        //@}

        /** @name Frame Control */
        //@{
        /** 
         * Request a new frame of rendering.
         *
         * This method is to be called only on the application node on an
         * initialized configuration.
         *
         * Dirty objects on the config are committed, i.e., the View, Canvas and
         * Observer. If the active Layout of a Canvas has been changed since the
         * last frame start, all outstanding rendering is completed using
         * finishAllFrames().
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
        EQ_EXPORT virtual uint32_t startFrame( const uint32_t frameID );

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
        EQ_EXPORT virtual uint32_t finishFrame();

        /**
         * Finish rendering all pending frames.
         *
         * This method is to be called only on the application node on an
         * initialized configuration.
         *
         * @return the frame number of the last finished frame.
         * @version 1.0
         */
        EQ_EXPORT virtual uint32_t finishAllFrames();

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
        //@}

        /** @name Event handling */
        //@{
        /** 
         * Send an event to the application node.
         *
         * @param event the event.
         * @version 1.0
         */
        EQ_EXPORT void sendEvent( ConfigEvent& event );

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
        EQ_EXPORT const ConfigEvent* nextEvent();

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
        EQ_EXPORT const ConfigEvent* tryNextEvent();

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
        EQ_EXPORT virtual void handleEvents();

        /** 
         * Handle one config event.
         * 
         * @param event the event.
         * @return true if the event requires a redraw, false if not.
         * @version 1.0
         */
        EQ_EXPORT virtual bool handleEvent( const ConfigEvent* event );
        //@}
        
        /** @name Error Information */
        //@{
        /** @return the error message from the last operation. @version 1.0 */
        const std::string& getErrorMessage() const { return _error; }
        //@}

        /** @internal */
        //@{
        /** 
         * Set up the config's message pump for the given pipe.
         * Used by non-threaded and AGL pipes.
         * @internal
         */
        void setupMessagePump( Pipe* pipe );
        //@}

    protected:

        /** @internal */
        //@{
        /** @internal */
        EQ_EXPORT virtual void notifyMapped( net::NodePtr node );
        //@}

    private:
        /** The node identifier of the node running the application thread. */
        net::NodeID _appNodeID;
        friend class Server;

        /** The name. */
        std::string _name;

        /** The node running the application thread. */
        net::NodePtr _appNode;

        /** Locally-instantiated nodes of this config. */
        NodeVector _nodes;

        /** The list of observers, app-node only. */
        ObserverVector _observers;

        /** The list of layouts, app-node only. */
        LayoutVector _layouts;

        /** The list of canvases, app-node only. */
        CanvasVector _canvases;

        /** The default distance between the left and the right eye. */
        float _eyeBase;

        /** The reason for the last error. */
        std::string _error;

        /** The receiver->app thread event queue. */
        CommandQueue _eventQueue;
        
        /** The last received event to be released. */
        net::Command* _lastEvent;

        /** Global statistics events, index per frame and channel. */
        std::deque< FrameStatistics > _statistics;
        base::Lock                    _statisticsMutex;

        /** The maximum number of outstanding frames. */
        uint32_t _latency;
        
        /** The last started frame. */
        uint32_t _currentFrame;
        /** The last locally released frame. */
        uint32_t _unlockedFrame;
        /** The last completed frame. */
        base::Monitor< uint32_t > _finishedFrame;

        /** The global clock. */
        base::Clock _clock;

        std::deque< int64_t > _frameTimes; //!< Start time of last frames

        /** true while the config is initialized and no window has exited. */
        bool _running;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        friend class Node;
        void _addNode( Node* node );
        void _removeNode( Node* node );
        Node* _findNode( const uint32_t id );
        void _frameStart();

        friend class ConfigDeserializer;
        void _addObserver( Observer* observer );
        void _removeObserver( Observer* observer );
        void _addLayout( Layout* layout );
        void _removeLayout( Layout* layout );
        void _addCanvas( Canvas* canvas );
        void _removeCanvas( Canvas* canvas );

        bool _needsLocalSync() const;

        /** 
         * Update statistics for the finished frame, push it to the local node
         * for visualization.
         */
        void _updateStatistics( const uint32_t finishedFrame );

        /** Init the application node instance */
        void _initAppNode( const uint32_t distributorID );

        /** Exit the current message pump */
        void _exitMessagePump();

        /** The command functions. */
        net::CommandResult _cmdSyncClock( net::Command& command );
        net::CommandResult _cmdCreateNode( net::Command& command );
        net::CommandResult _cmdDestroyNode( net::Command& command );
        net::CommandResult _cmdInitReply( net::Command& command );
        net::CommandResult _cmdExitReply( net::Command& command );
        net::CommandResult _cmdStartFrameReply( net::Command& command );
        net::CommandResult _cmdReleaseFrameLocal( net::Command& command );
        net::CommandResult _cmdFrameFinish( net::Command& command );
        net::CommandResult _cmdUnmap( net::Command& command );
    };
}

#endif // EQ_CONFIG_H

