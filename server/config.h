
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/net/session.h>

#include <iostream>
#include <vector>

namespace eq
{
namespace server
{
    class ConfigSerializer;
    class ConfigVisitor;
    class ConstConfigVisitor;
    struct CanvasPath;
    struct ChannelPath;
    struct LayoutPath;
    struct ObserverPath;
    struct SegmentPath;
    struct ViewPath;

    /**
     * The config.
     */
    class Config : public net::Session
    {
    public:
        /** 
         * Constructs a new Config.
         */
        EQSERVER_EXPORT Config();
        virtual ~Config();

        /** 
         * Constructs a new deep copy of a config.
         */
        Config( const Config& from );
        
        /**
         * @name Data Access.
         */
        //@{
        Server* getServer() { return _server; }
        
        Channel* getChannel( const ChannelPath& path );
        Canvas* getCanvas( const CanvasPath& path );
        Segment* getSegment( const SegmentPath& path );
        Layout* getLayout( const LayoutPath& path );
        Observer* getObserver( const ObserverPath& path );
        View* getView( const ViewPath& path );

        bool    isRunning() const { return ( _state == STATE_RUNNING ); }

        net::CommandQueue* getServerThreadQueue()
            { EQASSERT( _server ); return _server->getServerThreadQueue(); }
        
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
         * Adds a new observer to this config.
         * 
         * @param observer the observer.
         */
        void addObserver( Observer* observer );

        /** 
         * Removes a observer from this config.
         * 
         * @param observer the observer
         * @return <code>true</code> if the observer was removed,
         *         <code>false</code> otherwise.
         */
        bool removeObserver( Observer* observer );

        /** @return the vecotr of observers. */
        const ObserverVector& getObservers() const { return _observers; }

        /** 
         * Find the first observer of a given name.
         * 
         * @param name the name of the observer to find
         * @return the first observer with the name, or <code>0</code> if no
         *         observer with the name exists.
         */
        Observer* findObserver( const std::string& name );
        const Observer* findObserver( const std::string& name ) const;

        /** @return the observer mapped to the given identifier, or 0. */
        Observer* findObserver( const uint32_t id );

        /** 
         * Adds a new layout to this config.
         * 
         * @param layout the layout.
         */
        EQSERVER_EXPORT void addLayout( Layout* layout );

        /** 
         * Removes a layout from this config.
         * 
         * @param layout the layout
         * @return <code>true</code> if the layout was removed,
         *         <code>false</code> otherwise.
         */
        bool removeLayout( Layout* layout );

        /** @return the vecotr of layouts. */
        const LayoutVector& getLayouts() const { return _layouts; }

        /** 
         * Find the first layout of a given name.
         * 
         * @param name the name of the layout to find
         * @return the first layout with the name, or <code>0</code> if no
         *         layout with the name exists.
         */
        Layout* findLayout( const std::string& name );
        const Layout* findLayout( const std::string& name ) const;

        /** @return the layout mapped to the given identifier, or 0. */
        Layout* findLayout( const uint32_t id );

        /** 
         * Find the first view of a given name.
         * 
         * @param name the name of the view to find
         * @return the first view with the name, or <code>0</code> if no
         *         view with the name exists.
         */
        View* findView( const std::string& name );
        const View* findView( const std::string& name ) const;

        /** 
         * Adds a new canvas to this config.
         * 
         * @param canvas the canvas.
         */
        EQSERVER_EXPORT void addCanvas( Canvas* canvas );

        /** 
         * Removes a canvas from this config.
         * 
         * @param canvas the canvas
         * @return <code>true</code> if the canvas was removed,
         *         <code>false</code> otherwise.
         */
        bool removeCanvas( Canvas* canvas );

        /** @return the vecotr of canvases. */
        const CanvasVector& getCanvases() const { return _canvases; }

        /** 
         * Find the first canvas of a given name.
         * 
         * @param name the name of the canvas to find
         * @return the first canvas with the name, or <code>0</code> if no
         *         canvas with the name exists.
         */
        Canvas* findCanvas( const std::string& name );
        const Canvas* findCanvas( const std::string& name ) const;

        /** 
         * Find the first segment of a given name.
         * 
         * @param name the name of the segment to find
         * @return the first segment with the name, or <code>0</code> if no
         *         segment with the name exists.
         */
        Segment* findSegment( const std::string& name );
        const Segment* findSegment( const std::string& name ) const;

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
        EQSERVER_EXPORT Channel* findChannel( const std::string& name );
        const Channel* findChannel( const std::string& name ) const;

        /** 
         * Find the channel for the given view/segment intersection.
         * 
         * @param segment the segment.
         * @param view the view.
         * @return the channel for updating the view/segment intersection.
         */
        EQSERVER_EXPORT Channel* findChannel( const Segment* segment, const View* view );

        /** 
         * Traverse this config and all children using a config visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQSERVER_EXPORT VisitorResult accept( ConfigVisitor& visitor );
        VisitorResult accept( ConstConfigVisitor& visitor ) const;
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

    protected:
        /** @sa net::Session::notifyMapped. */
        virtual void notifyMapped( net::NodePtr node );

    private:
        /** The config name */
        std::string _name;

        /** The initID for late initialization. */
        uint32_t _initID;
        
        /** float attributes. */
        float _fAttributes[FATTR_ALL];

        /** String representation of float attributes. */
        static std::string _fAttributeStrings[FATTR_ALL];

        /** The eq server hosting the session. */
        Server* _server;
        friend class Server;

        /** The list of nodes. */
        NodeVector _nodes;

        /** The list of observers. */
        ObserverVector _observers;

        /** The list of layouts. */
        LayoutVector _layouts;

        /** The list of canvases. */
        CanvasVector _canvases;

        /** The list of compounds. */
        CompoundVector _compounds;

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

        enum State
        {
            STATE_STOPPED = 0,  // next: INITIALIZING
            STATE_INITIALIZING, // next: RUNNING
            STATE_RUNNING,      // next: EXITING
            STATE_EXITING,      // next: STOPPED
        }
        _state;

        ConfigSerializer* _serializer;

        /** The global clock. */
        base::Clock _clock;
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

        bool _startFrame( const uint32_t frameID );
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
    };

    std::ostream& operator << ( std::ostream& os, const Config* config );
}
}
#endif // EQSERVER_CONFIG_H
