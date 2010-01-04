
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/client/types.h>
#include <eq/client/visitorResult.h>  // enum

#include <eq/net/barrier.h>
#include <eq/net/objectVersion.h>
#include <eq/base/base.h>

namespace eq
{
    class CommandQueue;
    class FrameData;
    class NodeVisitor;

    /**
     * A Node represents a single computer in the cluster.
     *
     * Each node is executed in a seperate process.
     */
    class Node : public net::Object
    {
    public:
        /** Constructs a new node. */
        EQ_EXPORT Node( Config* parent );

        /** Destructs the node. */
        EQ_EXPORT virtual ~Node();

        /** 
         * Returns the config of this node.
         * 
         * @return the config of this node. 
         */
        Config*       getConfig()       { return _config; }
        const Config* getConfig() const { return _config; }

        EQ_EXPORT ClientPtr getClient();
        EQ_EXPORT ServerPtr getServer();

        const PipeVector& getPipes() const { return _pipes; }
        const std::string& getName() const { return _name; }

        /** 
         * Return the set of tasks this nodes's channels might execute in the
         * worst case.
         * 
         * It is not guaranteed that all the tasks will be actually executed
         * during rendering.
         * 
         * @warning Not finalized, might change in the future.
         * @return a bitwise combination of the Task enum.
         */
        uint32_t getTasks() const { return _tasks; }

        EQ_EXPORT CommandQueue* getNodeThreadQueue();

        /** 
         * Traverse this node and all children using a node visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQ_EXPORT VisitorResult accept( NodeVisitor& visitor );

        /** 
         * Get a network barrier. 
         * 
         * @param barrier the barrier identifier and version.
         * @return the barrier.
         * @internal
         */
        net::Barrier* getBarrier( const net::ObjectVersion barrier );

        /** 
         * Get a frame data instance.
         * 
         * @param dataVersion the frame data identifier and version.
         * @return the frame.
         * @internal
         */
        FrameData* getFrameData( const net::ObjectVersion& dataVersion );

        /** Wait for the node to be initialized. */
        EQ_EXPORT void waitInitialized() const;
        EQ_EXPORT bool isRunning() const;
        
        /** 
         * Wait for a frame to be started.
         * 
         * @param frameNumber the frame number.
         * @sa releaseFrame()
         */
        EQ_EXPORT void waitFrameStarted( const uint32_t frameNumber ) const;

        uint32_t getFinishedFrame() const { return _finishedFrame; }

        /**
         * @name Attributes
         */
        //@{
        // Note: also update string array initialization in node.cpp
        /** Node attributes. */
        enum IAttribute
        {
            /** <a href="http://www.equalizergraphics.com/documents/design/threads.html#sync">Threading model</a> */
            IATTR_THREAD_MODEL,
            IATTR_LAUNCH_TIMEOUT,         //!< Launch timeout
            IATTR_FILL1,
            IATTR_FILL2,
            IATTR_ALL
        };

        EQ_EXPORT void setIAttribute( const IAttribute attr,
                                      const int32_t value );
        EQ_EXPORT int32_t getIAttribute( const IAttribute attr ) const;
        EQ_EXPORT static const std::string& getIAttributeString(
                                                        const IAttribute attr );
        //@}

        class TransmitThread : public base::Thread
        {
        public:
            TransmitThread( Node* parent ) : _node( parent ) {}
            virtual ~TransmitThread() {}

            void send( FrameData* data, net::NodePtr node, 
                       const uint32_t frameNumber );
            
        protected:
            virtual void* run();

        private:
            struct Task
            {
                Task( FrameData* d, net::NodePtr n, const uint32_t f ) 
                        : data( d ), node( n ), frameNumber( f ) {}

                FrameData*   data;
                net::NodePtr node;
                uint32_t     frameNumber;
            };

            base::MTQueue< Task > _tasks;
            Node* const           _node;
        };

        TransmitThread transmitter;

    protected:
        friend class Config;

        EQ_EXPORT virtual void attachToSession( const uint32_t id, 
                                                const uint32_t instanceID, 
                                                net::Session* session );

        /** @name Actions */
        //@{
        /** 
         * Start a frame by unlocking all child resources.
         * 
         * @param frameNumber the frame to start.
         */
        EQ_EXPORT void startFrame( const uint32_t frameNumber );

        /** 
         * Signal the completion of a frame to the parent.
         * 
         * @param frameNumber the frame to end.
         */
        EQ_EXPORT void releaseFrame( const uint32_t frameNumber );

        /** 
         * Release the local synchronization of the parent for a frame.
         * 
         * @param frameNumber the frame to release.
         */
        EQ_EXPORT void releaseFrameLocal( const uint32_t frameNumber );
        //@}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialises this node.
         * 
         * @param initID the init identifier.
         */
        EQ_EXPORT virtual bool configInit( const uint32_t initID );

        /** 
         * Exit this node.
         */
        EQ_EXPORT virtual bool configExit();

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
         */
        EQ_EXPORT virtual void frameStart( const uint32_t frameID, 
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
         */
        virtual void frameFinish( const uint32_t frameID, 
                                  const uint32_t frameNumber ) 
            { releaseFrame( frameNumber ); }

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
         */
        EQ_EXPORT virtual void frameDrawFinish( const uint32_t frameID, 
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
         */
        EQ_EXPORT virtual void frameTasksFinish( const uint32_t frameID, 
                                                 const uint32_t frameNumber );
        //@}

        /** @name Error information. */
        //@{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within the init method.
         *
         * @param message the error message.
         */
        EQ_EXPORT void setErrorMessage( const std::string& message );
        //@}

    private:
        /** The parent config */
        Config* const          _config;

        /** The name. */
        std::string            _name;

        /** Integer attributes. */
        int32_t _iAttributes[IATTR_ALL];
        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** Pipe children. */
        PipeVector             _pipes;

        /** The reason for the last error. */
        std::string            _error;

        enum State
        {
            STATE_STOPPED,
            STATE_INITIALIZING,
            STATE_INIT_FAILED,
            STATE_RUNNING
        };
        /** The configInit/configExit state. */
        base::Monitor< State > _state;

        /** The number of the last started frame. */
        base::Monitor<uint32_t> _currentFrame;

        /** The number of the last locally released frame. */
        uint32_t _unlockedFrame;

        /** The number of the last finished frame. */
        uint32_t _finishedFrame;

        typedef stde::hash_map< uint32_t, net::Barrier* > BarrierHash;
        /** All barriers mapped by the node. */
        BarrierHash _barriers;
        base::Lock  _barriersMutex;

        typedef stde::hash_map< uint32_t, FrameData* > FrameDataHash;
        /** All frame datas used by the node during rendering. */
        FrameDataHash _frameDatas;
        base::Lock    _frameDatasMutex;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        friend class Pipe;
        void _addPipe( Pipe* pipe );
        void _removePipe( Pipe* pipe );
        Pipe* _findPipe( const uint32_t id );

        void _finishFrame( const uint32_t frameNumber ) const;
        void _frameFinish( const uint32_t frameID, const uint32_t frameNumber );

        void _flushObjects();

        virtual void getInstanceData( net::DataOStream& os ) { EQDONTCALL }
        virtual void applyInstanceData( net::DataIStream& is ) { EQDONTCALL }

        /** The command functions. */
        net::CommandResult _cmdCreatePipe( net::Command& command );
        net::CommandResult _cmdDestroyPipe( net::Command& command );
        net::CommandResult _cmdConfigInit( net::Command& command );
        net::CommandResult _cmdConfigExit( net::Command& command );
        net::CommandResult _cmdFrameStart( net::Command& command );
        net::CommandResult _cmdFrameFinish( net::Command& command );
        net::CommandResult _cmdFrameDrawFinish( net::Command& command );
        net::CommandResult _cmdFrameTasksFinish( net::Command& command );

        CHECK_THREAD_DECLARE( _nodeThread );
    };
}

#endif // EQ_NODE_H

