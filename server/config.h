
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
    class ConfigVisitor;
    class Layout;
    class Observer;

    /**
     * The config.
     */
    class Config : public fabric::Config< Server, Config, Observer, Layout,
                                          Canvas, Node, ConfigVisitor >
    {
    public:
        typedef fabric::Config< Server, Config, Observer, Layout, Canvas, Node,
                                ConfigVisitor > Super;

        /** Construct a new config. */
        EQSERVER_EXPORT Config( ServerPtr parent );
        virtual ~Config();

        /**
         * @name Data Access.
         */
        //@{
        Channel* getChannel( const ChannelPath& path );
        Segment* getSegment( const SegmentPath& path );
        View* getView( const ViewPath& path );

        bool isRunning() const { return ( _state == STATE_RUNNING ); }
        bool isUsed() const { return _appNetNode.isValid(); }

        net::CommandQueue* getMainThreadQueue()
            { return getServer()->getMainThreadQueue(); }
        
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
        const Compounds& getCompounds() const { return _compounds; }

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

        /** @return the application node, or 0. */
        EQSERVER_EXPORT Node* findApplicationNode();
        //@}

        /** @sa fabric::Config::changeLatency() */
        virtual void changeLatency( const uint32_t latency );

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
         * Register all shared objects of the session and return the identifier
         * of the config proxy.
         */
        uint32_t register_();

        /** 
         * Deregister all shared objects of the session, called before session
         * teardown.
         */
        void deregister();
        
        virtual void restore();

        /** Return the initID for late initialization  */
        uint32_t getInitID(){ return _initID; }

        /** Activate the given canvas after it is complete (dest channels). */
        virtual void activateCanvas( Canvas* canvas );

        /** Initialize the given canvas in a running configuration */
        virtual void updateCanvas( Canvas* canvas );

        /** Request a finish of outstanding frames on next frame */
        void postNeedsFinish() { _needsFinish = true; }

        /** @internal */
        virtual VisitorResult _acceptCompounds( ConfigVisitor& visitor );
        /** @internal */
        virtual VisitorResult _acceptCompounds( ConfigVisitor& visitor ) const;

        void output( std::ostream& os ) const; //!< @internal
        virtual bool mapViewObjects() const { return true; } //!< @internal
        virtual bool mapNodeObjects() const { return true; } //!< @internal

    protected:
        /** @sa net::Session::notifyMapped. */
        virtual void notifyMapped( net::NodePtr node );

        friend class Server; // for commit()

    private:
        Config( const Config& from );

        /** The initID for late initialization. */
        uint32_t _initID;

        /** The list of compounds. */
        Compounds _compounds;

        /** The network node running the application thread. */
        net::NodePtr _appNetNode;

        /** The name of the render client executable. */
        std::string _renderClient;

        /** The working directory of the render client. */
        std::string _workDir;

        /** The last started frame, or 0. */
        uint32_t _currentFrame;

        /** The last finished frame, or 0. */
        base::Monitor< uint32_t > _finishedFrame;

        enum State
        {
            STATE_STOPPED = 0,  // next: INITIALIZING
            STATE_INITIALIZING, // next: RUNNING
            STATE_RUNNING,      // next: EXITING
            STATE_EXITING,      // next: STOPPED
        }
        _state;

        bool _needsFinish; //!< true after runtime changes

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /**
         * @name Operations
         */
        //@{
        bool _updateRunning(); //!< @return true on success, false on error

        void _updateCanvases();
        bool _connectNodes();
        bool _connectNode( Node* node );
        bool _syncConnectNode( Node* node, const base::Clock& clock );
        void _startNodes();
        uint32_t _createConfig( Node* node );
        bool _updateNodes();
        void _stopNodes();
        template< class T >
        void _deleteEntities( const std::vector< T* >& entities );
        void _syncClock();

        bool _init( const uint32_t initID );

        void _startFrame( const uint32_t frameID );
        void _flushAllFrames();
        //@}

        virtual Observer* createObserver();
        virtual void releaseObserver( Observer* observer );
        virtual Layout* createLayout();
        virtual void releaseLayout( Layout* layout );
        virtual Canvas* createCanvas();
        virtual void releaseCanvas( Canvas* canvas );

        /** @internal Execute the slave remove request. */
        virtual void _removeChild( const uint32_t id );

        /** @internal Post deletion for the given child, returns true if found*/
        template< class T > bool _postDelete( const uint32_t id );

        /** The command functions. */
        bool _cmdInit( net::Command& command );
        bool _cmdExit( net::Command& command );
        bool _cmdStartFrame( net::Command& command );
        bool _cmdFinishAllFrames( net::Command& command ); 
        bool _cmdCreateReply( net::Command& command );
        bool _cmdFreezeLoadBalancing( net::Command& command );
    };
}
}
#endif // EQSERVER_CONFIG_H
