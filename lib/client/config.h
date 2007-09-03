
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIG_H
#define EQ_CONFIG_H

#include <eq/client/client.h>       // called in inline method
#include <eq/client/commandQueue.h> // member
#include <eq/client/matrix4.h>      // member
#include <eq/net/session.h>         // base class


namespace eq
{
    class Client;
    class Node;
    class SceneObject;
    class Server;
    struct ConfigEvent;

    class EQ_EXPORT Config : public eqNet::Session
    {
    public:
        /** 
         * Constructs a new config.
         */
        Config();

        /** @name Data Access */
        //*{
        eqBase::RefPtr< Client > getClient();
        const std::vector< Node* >& getNodes() const { return _nodes; }
        //*}

        /** 
         * Initializes this configuration.
         * 
         * @param initID an identifier to be passed to all init methods.
         * @return true if the initialisation was successful, false if not.
         */
        virtual bool init( const uint32_t initID )
            { return _startInit( initID ) && _finishInit(); }

        /** 
         * Exits this configuration.
         * 
         * A config which could not be exited properly may not be
         * re-initialized. The exit function does not provide the possibility to
         * pass an exit identifier to the exit methods, because individual
         * entities may stopped dynamically by the server when running a config,
         * i.e., before exit() is called.
         *
         * @return <code>true</code> if the exit was successful,
         *         <code>false</code> if not.
         */
        virtual bool exit();

        /**
         * @name Frame Control
         */
        //*{
        /** 
         * Requests a new frame of rendering.
         * 
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @return the frame number of the new frame.
         */
        virtual uint32_t startFrame( const uint32_t frameID );

        /** 
         * Sends frame data to 
         * 
         * @param data 
         */
        void sendSceneObject( SceneObject* object );
        void flushSceneObjects();

        /** 
         * Finish the rendering of a frame.
         * 
         * @return the frame number of the finished frame, or <code>0</code> if
         *         no frame has been finished.
         */
        virtual uint32_t finishFrame();

        /**
         * Finish rendering all pending frames.
         *
         * @return the frame number of the last finished frame.
         */
        virtual uint32_t finishAllFrames();
        //*}

        /** @name Event handling. */
        //*{
        /** 
         * Send an event to the application node.
         * 
         * @param event the event.
         */
        void sendEvent( ConfigEvent& event );

        /** 
         * Get the next received event on the application node.
         * 
         * The returned event is valid until the next call to this method.
         * 
         * @return a config event.
         */
        const ConfigEvent* nextEvent();

        /** @return true if events are pending. */
        bool checkEvent() const { return !_eventQueue.empty(); }

        /**
         * Handle all config events.
         *
         * Called at the end of each frame to handle pending config events. The
         * default implementation calls handleEvent() on all pending events,
         * without blocking.
         */
        virtual void handleEvents();

        /** 
         * Handle one config event.
         * 
         * @param event the event.
         * @return <code>true</code> if the event was handled,
         *         <code>false</code> if not.
         */
        virtual bool handleEvent( const ConfigEvent* event ){ return false; }
        //*}

        /** Sets the head matrix according to the specified matrix.
         *
         * @param matrix the matrix
         */
        void setHeadMatrix( const vmml::Matrix4f& matrix );

        /** @name Error Information. */
        //*{
        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        //*}

#ifdef EQ_TRANSMISSION_API
        /** @name Data Transmission. */
        //*{
        /** 
         * Send data to all active render client nodes.
         * 
         * @param data the pointer to the data.
         * @param size the data size.
         */
        void broadcastData( const void* data, uint64_t size );
        //*}
#endif

    protected:
        virtual ~Config();

    private:
        /** The node identifier of the node running the application thread. */
        eqNet::NodeID _appNodeID;
        friend class Server;

        /** The node running the application thread. */
        eqBase::RefPtr<eqNet::Node> _appNode;

#ifdef EQ_TRANSMISSION_API
        /** The list of the running client node identifiers. */
        std::vector<eqNet::NodeID> _clientNodeIDs;

        /** The running client nodes, is cleared when _clientNodeIDs changes. */
        std::vector< eqBase::RefPtr<eqNet::Node> > _clientNodes;
#endif

        /** Locally-instantiated nodes of this config. */
        std::vector<Node*> _nodes;

        void _addNode( Node* node );
        void _removeNode( Node* node );
        Node* _findNode( const uint32_t id );

        /** 
         * Start initializing the configuration.
         * 
         * @param initID an identifier to be passed to all init methods. 
         */
        virtual bool _startInit( const uint32_t initID );

        /** 
         * Finish initializing the configuration.
         * 
         * @return true if the initialisation was successful, false if not.
         */
        virtual bool _finishInit();

        /** The matrix describing the head position and orientation. */
        Matrix4f _headMatrix;

        /** The reason for the last error. */
        std::string            _error;

        /** Registers pending commands waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** The receiver->app thread event queue. */
        CommandQueue           _eventQueue;

#ifdef EQ_TRANSMISSION_API
        /** Connect client nodes of this config. */
        bool _connectClientNodes();
#endif

        /** The command functions. */
        eqNet::CommandResult _cmdCreateNode( eqNet::Command& command );
        eqNet::CommandResult _cmdDestroyNode( eqNet::Command& command );
        eqNet::CommandResult _reqStartInitReply( eqNet::Command& command );
        eqNet::CommandResult _reqFinishInitReply( eqNet::Command& command );
        eqNet::CommandResult _reqExitReply( eqNet::Command& command );
        eqNet::CommandResult _cmdStartFrameReply( eqNet::Command& command );
        eqNet::CommandResult _reqFinishFrameReply( eqNet::Command& command );
        eqNet::CommandResult _reqFinishAllFramesReply( eqNet::Command& command);
        eqNet::CommandResult _cmdEvent( eqNet::Command& command );
#ifdef EQ_TRANSMISSION_API
        eqNet::CommandResult _cmdData( eqNet::Command& command );
#endif
    };
}

#endif // EQ_CONFIG_H

