
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CONFIG_H
#define EQSERVER_CONFIG_H

#include "base.h"
#include "types.h"
#include "server.h"        // used in inline method
#include "configVisitor.h" // nested enum

#include <eq/client/matrix4.h>
#include <eq/client/packets.h>
#include <eq/net/session.h>

#include <iostream>
#include <vector>

namespace eq
{
namespace server
{
    /**
     * The config.
     */
    class Config : public net::Session
    {
    public:
        /** 
         * Constructs a new Config.
         */
        Config();
        virtual ~Config();

        /** 
         * Constructs a new deep copy of a config.
         */
        Config( const Config& from );
        
        /**
         * @name Data Access.
         */
        //*{
        Server* getServer() { return _server.get(); }

        bool    isRunning() const { return ( _state == STATE_INITIALIZED ); }

        net::CommandQueue* getServerThreadQueue()
            { return _server->getServerThreadQueue(); }

        /** 
         * Adds a new node to this config.
         * 
         * @param node the node.
         */
        void addNode( Node* node );

        /** 
         * Removes a node from this config.
         * 
         * @param node the node
         * @return <code>true</code> if the node was removed, 
         *         <code>false</code> otherwise.
         */
        bool removeNode( Node* node );

        /** @return the vector of nodes. */
        const NodeVector& getNodes() const { return _nodes; }

        /** 
         * Adds a new compound to this config.
         * 
         * @param compound the compound.
         */
        void addCompound( Compound* compound );

        /** 
         * Removes a compound from this config.
         * 
         * @param compound the compound
         * @return <code>true</code> if the compound was removed,
         *         <code>false</code> otherwise.
         */
        bool removeCompound( Compound* compound );

        /** @return the vecotr of compounds. */
        const CompoundVector& getCompounds() const { return _compounds; }

        /** 
         * Find the first channel of a given name.
         * 
         * @param name the name of the channel to find
         * @return the first channel with the name, or <code>0</code> if no
         *         channel with the name exists.
         */
        Channel* findChannel( const std::string& name );

        /** 
         * Traverse this config and all children using a config visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        ConfigVisitor::Result accept( ConfigVisitor* visitor );
        //*}

        /** 
         * Set the maximum accepted latency for this config.
         * 
         * The latency is defined as the maximum number of frames between the
         * start of a frame and the finish of the last rendering task for that
         * frame.
         *
         * @param latency the latency.
         */
        void setLatency( const uint32_t latency ) { _latency = latency; }

        /** 
         * Return the latency for this config.
         * @return the latency for this config.
         */
        uint32_t getLatency() const { return _latency; }

        /** 
         * Set the name of the application.
         * 
         * @param name the name of the application.
         */
        void setApplicationName( const std::string& name )  { _appName = name; }

        /** 
         * Add the config node running the application thread.
         * 
         * @param node the application node.
         */
        void addApplicationNode( Node* node );

        /** @return the application node, or 0 if it was not set. */
        Node* getApplicationNode() { return _appNode; }

        /** @return true if the node is the application node. */
        bool isApplicationNode( const Node* node ) const
            { return (_appNode == node); }

        /** 
         * Set the network node running the application thread.
         * 
         * @param node the application node.
         */
        void setApplicationNetNode( net::NodePtr node );

        /** 
         * Set the name of the render client executable. 
         * 
         * @param rc the name of the render client executable.
         */
        void setRenderClient( const std::string& rc ){ _renderClient = rc; }

        /** 
         * Set the working directory for render client.
         * 
         * @param workDir the working directory for the  render client.
         */
        void setWorkDir( const std::string& workDir ){ _workDir = workDir; }

         /** 
          * Return the position of the eye in world-space.
          * 
          * @return the eye position.
          */
        const vmml::Vector3f& getEyePosition( const eq::Eye eye ) const
            { return _eyePosition[ eye ]; }

        /** Notify that a node of this config has finished a frame. */
        void notifyNodeFrameFinished( const uint32_t frameNumber );

        // Used by Server::releaseConfig() to make sure config is exited
        bool exit();

        /**
         * @name Float Attributes
         */
        // Note: also update string array initialization in config.cpp
        enum FAttribute
        {
            FATTR_EYE_BASE,
            FATTR_ALL
        };
        
        void setFAttribute( const FAttribute attr, const float value )
            { _fAttributes[attr] = value; }
        float getFAttribute( const FAttribute attr ) const
            { return _fAttributes[attr]; }
        static const std::string&  getFAttributeString( const FAttribute attr )
            { return _fAttributeStrings[attr]; }

        /** @name Error information. */
        //@{
        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        //@}

    protected:
        /** @sa net::Session::setLocalNode. */
        virtual void setLocalNode( net::NodePtr node );

    private:
        /** float attributes. */
        float _fAttributes[FATTR_ALL];
        /** String representation of float attributes. */
        static std::string _fAttributeStrings[FATTR_ALL];
        
        /** The eq server hosting the session. */
        ServerPtr _server;
        friend class Server;

        /** The list of compounds. */
        CompoundVector _compounds;

        /** The list of nodes. */
        NodeVector _nodes;

        /** The reason for the last error. */
        std::string            _error;

        /** The name of the application. */
        std::string _appName;

        /** The node running the application thread. */
        Node*       _appNode;

        /** The network node running the application thread. */
        net::NodePtr _appNetNode;

        /** The name of the render client executable. */
        std::string _renderClient;

        /** The working directory of the render client. */
        std::string _workDir;

        /** The maximum latency between frame start and end frame, in frames. */
        uint32_t _latency;

        /** The last started frame, or 0. */
        uint32_t _currentFrame;

        /** The last finished frame, or 0. */
        uint32_t _finishedFrame;

        /** The matrix defining the head's position for head tracking. */
        eq::Matrix4f _headMatrix;

        /** The vectors defining the eye positions. */
        vmml::Vector3f _eyePosition[eq::EYE_ALL];

        enum State
        {
            STATE_STOPPED,
            STATE_INITIALIZED
        }
            _state;

        /**
         * @name Operations
         */
        //*{
        /** common code for all constructors */
        void _construct();

        bool _startInit( const uint32_t initID, 
                         std::vector< net::NodeID::Data >& nodeIDs );
        bool   _connectNodes();
        bool   _initNodes( const uint32_t initID, 
                           std::vector< net::NodeID::Data >& nodeIDs );
        bool _finishInit();

        bool _exitNodes();

        void _updateHead();

        void _prepareFrame( std::vector< net::NodeID >& nodeIDs );
        void _startFrame( const uint32_t frameID );
        void _flushFrames();
        //*}

        /** The command functions. */
        net::CommandResult _cmdStartInit( net::Command& command );
        net::CommandResult _cmdFinishInit( net::Command& command );
        net::CommandResult _cmdExit( net::Command& command );
        net::CommandResult _cmdStartFrame( net::Command& command );
        net::CommandResult _cmdFinishAllFrames( net::Command& command ); 
        net::CommandResult _cmdCreateReply( net::Command& command );
        net::CommandResult _cmdCreateNodeReply( net::Command& command );
    };

    std::ostream& operator << ( std::ostream& os, const Config* config );
}
}
#endif // EQSERVER_CONFIG_H
