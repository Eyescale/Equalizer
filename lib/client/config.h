
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
#include <eq/client/types.h>         // typedefs
#include <eq/client/visitorResult.h> // enum

#include <eq/fabric/config.h>        // base class
#include <eq/base/monitor.h>         // member

namespace eq
{
    class Layout;
    class Node;
    class Observer;
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
    class Config : public fabric::Config< Server, Config, Observer, Layout,
                                          Canvas, Node, ConfigVisitor >
    {
    public:
        /** Construct a new config. @version 1.0 */
        EQ_EXPORT Config( ServerPtr parent );

        /** Destruct a config. @version 1.0 */
        EQ_EXPORT virtual ~Config();

        /** @name Data Access */
        //@{
        /** @return the local client node. @version 1.0 */
        EQ_EXPORT ClientPtr getClient();

        /** @internal */
        EQ_EXPORT CommandQueue* getMainThreadQueue();

        /** @return the frame number of the last frame started. @version 1.0 */
        uint32_t getCurrentFrame()  const { return _currentFrame; }

        /** @return the frame number of the last frame finished. @version 1.0 */
        uint32_t getFinishedFrame() const { return _finishedFrame.get(); }

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

        /** @sa fabric::Config::setLatency() */
        EQ_EXPORT virtual void setLatency( const uint32_t latency );
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
        EQ_EXPORT virtual void notifyMapped( net::NodePtr node );
        EQ_EXPORT virtual void changeLatency( const uint32_t latency );
        EQ_EXPORT virtual void unmap(); //!< @internal
        template< class, class, class, class > friend class fabric::Server;
        EQ_EXPORT virtual bool mapViewObjects(); //!< @internal
        //@}

    private:
        /** The node running the application thread. */
        net::NodePtr _appNode;

        /** The receiver->app thread event queue. */
        CommandQueue _eventQueue;
        
        /** The last received event to be released. */
        net::Command* _lastEvent;

        /** Global statistics events, index per frame and channel. */
        std::deque< FrameStatistics > _statistics;
        base::Lock                    _statisticsMutex;
        
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

        /** Exit the current message pump */
        void _exitMessagePump();

        /** The command functions. */
        net::CommandResult _cmdSyncClock( net::Command& command );
        net::CommandResult _cmdCreateNode( net::Command& command );
        net::CommandResult _cmdDestroyNode( net::Command& command );
        net::CommandResult _cmdStartFrameReply( net::Command& command );
        net::CommandResult _cmdInitReply( net::Command& command );
        net::CommandResult _cmdExitReply( net::Command& command );
        net::CommandResult _cmdReleaseFrameLocal( net::Command& command );
        net::CommandResult _cmdFrameFinish( net::Command& command );
    };
}

#endif // EQ_CONFIG_H

