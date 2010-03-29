
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

#ifndef EQSERVER_CONFIG_H
#define EQSERVER_CONFIG_H

#include "base.h"
#include "types.h"
#include "server.h"        // used in inline method
#include "visitorResult.h" // enum

#include <eq/fabric/config.h> // base class

#include <iostream>
#include <vector>

namespace eq
{
namespace server
{
    class ConfigSerializer;
    class ConfigVisitor;
    class Layout;
    class Observer;

    /**
     * The config.
     */
    class Config : public fabric::Config< Server, Config, Observer, Layout,
                                          Canvas >
    {
    public:
        /** Construct a new config. */
        EQSERVER_EXPORT Config( ServerPtr parent );
        virtual ~Config();

        /** Constructs a new deep copy of a config. */
        Config( const Config& from, ServerPtr parent );
        
        /**
         * @name Data Access.
         */
        //@{
        Channel* getChannel( const ChannelPath& path );
        Segment* getSegment( const SegmentPath& path );
        View* getView( const ViewPath& path );

        bool    isRunning() const { return ( _state == STATE_RUNNING ); }

        net::CommandQueue* getServerThreadQueue()
            { return getServer()->getServerThreadQueue(); }
        
        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        /** 
         * Adds a new node to this config.
         * 
         * @param node the node.
         */
        EQSERVER_EXPORT void addNode( Node* node );

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
        EQSERVER_EXPORT void addCompound( Compound* compound );

        /** 
         * Removes a compound from this config.
         * 
         * @param compound the compound
         * @return <code>true</code> if the compound was removed,
         *         <code>false</code> otherwise.
         */
       EQSERVER_EXPORT bool removeCompound( Compound* compound );

        /** @return the vecotr of compounds. */
        const CompoundVector& getCompounds() const { return _compounds; }

        /** 
         * Find the first channel of a given name.
         * 
         * @param name the name of the channel to find
         * @return the first channel with the name, or <code>0</code> if no
         *         channel with the name exists.
         */
        const Channel* findChannel( const std::string& name ) const;

        /** 
         * Find the channel for the given view/segment intersection.
         * 
         * @param segment the segment.
         * @param view the view.
         * @return the channel for updating the view/segment intersection.
         */
        EQSERVER_EXPORT Channel* findChannel( const Segment* segment,
                                              const View* view );

        /** 
         * Traverse this config and all children using a config visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQSERVER_EXPORT VisitorResult accept( ConfigVisitor& visitor );
        VisitorResult accept( ConfigVisitor& visitor ) const;
        //@}

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

        /** @return the latency for this config. */
        uint32_t getLatency() const { return _latency; }

        /** @return the last finished frame. */
        uint32_t getFinishedFrame() const { return _finishedFrame; }
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

        /** Notify that a node of this config has finished a frame. */
        void notifyNodeFrameFinished( const uint32_t frameNumber );

        // Used by Server::releaseConfig() to make sure config is exited
        bool exit();

        /** 
         * Unmap all shared objects of the session, called before session
         * teardown.
         */
        void unmap();
        
        /** @name Attributes */
        //@{
        // Note: also update string array initialization in config.cpp
        enum FAttribute
        {
            FATTR_EYE_BASE,
            FATTR_VERSION,
            FATTR_FILL1,
            FATTR_FILL2,
            FATTR_ALL
        };
        
        void setFAttribute( const FAttribute attr, const float value )
            { _fAttributes[attr] = value; }
        float getFAttribute( const FAttribute attr ) const
            { return _fAttributes[attr]; }
        static const std::string&  getFAttributeString( const FAttribute attr )
            { return _fAttributeStrings[attr]; }
        //@}
 
        /** @name Error information. */
        //@{
        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        //@}

        /** Used by server during appNode initialization. */
        uint32_t getDistributorID();

        /** Return the initID for late initialization  */
        uint32_t getInitID(){ return _initID; }

        /** Activate the given canvas after it is complete (dest channels) */
        virtual void activateCanvas( Canvas* canvas );

    protected:
        /** @sa net::Session::notifyMapped. */
        virtual void notifyMapped( net::NodePtr node );

    private:
        Config( const Config& from );

        /** The config name */
        std::string _name;

        /** The initID for late initialization. */
        uint32_t _initID;
        
        /** float attributes. */
        float _fAttributes[FATTR_ALL];

        /** String representation of float attributes. */
        static std::string _fAttributeStrings[FATTR_ALL];

        /** The list of nodes. */
        NodeVector _nodes;

        /** The list of compounds. */
        CompoundVector _compounds;

        /** The reason for the last error. */
        std::string _error;

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

        enum State
        {
            STATE_STOPPED = 0,  // next: INITIALIZING
            STATE_INITIALIZING, // next: RUNNING
            STATE_RUNNING,      // next: EXITING
            STATE_EXITING,      // next: STOPPED
        }
        _state;

        ConfigSerializer* _serializer;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        /**
         * @name Operations
         */
        //@{
        /** common code for all constructors */
        void _construct();

        bool _updateRunning();
        bool   _connectNodes();
        bool     _connectNode( Node* node );
        bool     _syncConnectNode( Node* node, const base::Clock& clock );
        void   _startNodes();
        uint32_t _createConfig( Node* node );
        void   _stopNodes();
        void   _syncClock();

        bool _init( const uint32_t initID );

        void _startFrame( const uint32_t frameID );
        void _flushAllFrames();
        //@}

        /** The command functions. */
        net::CommandResult _cmdInit( net::Command& command );
        net::CommandResult _cmdExit( net::Command& command );
        net::CommandResult _cmdStartFrame( net::Command& command );
        net::CommandResult _cmdFinishAllFrames( net::Command& command ); 
        net::CommandResult _cmdCreateReply( net::Command& command );
        net::CommandResult _cmdFreezeLoadBalancing( net::Command& command );
        net::CommandResult _cmdUnmapReply( net::Command& command );
        net::CommandResult _cmdChangeLatency( net::Command& command );
    };

    std::ostream& operator << ( std::ostream& os, const Config* config );
}
}
#endif // EQSERVER_CONFIG_H
