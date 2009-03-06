
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CHANNEL_H
#define EQSERVER_CHANNEL_H

#ifdef EQUALIZERSERVERLIBRARY_EXPORTS
   // We need to instantiate a Monitor< State > when compiling the library,
   // but we don't want to have <pthread.h> for a normal build, hence this hack
#  include <pthread.h>
#endif
#include <eq/base/monitor.h>

#include "base.h"
#include "types.h"
#include "visitorResult.h" // nested enum

#include <eq/client/channel.h>
#include <eq/client/commands.h>
#include <eq/client/pixelViewport.h>
#include <eq/client/viewport.h>
#include <eq/net/object.h>
#include <eq/net/packets.h>

#include <iostream>
#include <vector>

namespace eq
{

namespace server
{
    class ChannelListener;
    class ChannelVisitor;
    class ConstChannelVisitor;
    struct ChannelPath;

    /**
     * The channel.
     */
    class EQSERVER_EXPORT Channel : public net::Object
    {
    public:
        enum State
        {
            STATE_STOPPED = 0,  // next: INITIALIZING
            STATE_INITIALIZING, // next: INIT_FAILED or INIT_SUCCESS
            STATE_INIT_SUCCESS, // next: RUNNING
            STATE_INIT_FAILED,  // next: EXITING
            STATE_RUNNING,      // next: EXITING
            STATE_EXITING,      // next: EXIT_FAILED or EXIT_SUCCESS
            STATE_EXIT_SUCCESS, // next: STOPPED
            STATE_EXIT_FAILED,  // next: STOPPED
        };

        /** 
         * Constructs a new Channel.
         */
        Channel();

        /** 
         * Constructs a new deep copy of a channel.
         */
        Channel( const Channel& from, Window* window );

        /** Destruct this channel. */
        virtual ~Channel();

        /** 
         * @return the state of this channel.
         */
        State getState()    const { return _state.get(); }

        /**
         * @name Data Access
         */
        //*{
        Config* getConfig();
        const Config* getConfig() const;

        Node* getNode();
        const Node* getNode() const;

        Pipe* getPipe();
        const Pipe* getPipe() const;

        Window* getWindow()             { return _window; }
        const Window* getWindow() const { return _window; }

        /** @return the index path to this channel. */
        ChannelPath getPath() const;

        const CompoundVector& getCompounds() const;

        net::CommandQueue* getServerThreadQueue();
        net::CommandQueue* getCommandThreadQueue();

        /** 
         * Traverse this channel using a channel visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( ChannelVisitor& visitor );
        VisitorResult accept( ConstChannelVisitor& visitor ) const;

        /** Increase channel activition count. */
        void activate();

        /** Decrease channel activition count. */
        void deactivate();

        /** @return if this channel is actively used for rendering. */
        bool isActive() const { return (_active != 0); }

        /**
         * Add additional tasks this channel, and all its parents, might
         * potentially execute.
         */
        void addTasks( const uint32_t tasks );

        /** Set the view for this channel. */
        void setView( const View* view );
        /** @return the channel's view. */
        const View* getView() const { return _view; }

        /** @return the channel's layout. */
        const Layout* getLayout() const;

        /** Set the segment for this channel. */
        void setSegment( const Segment* segment );
        /** @return the channel's segment. */
        const Segment* getSegment() const { return _segment; }

        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        /** 
         * Set the channel's pixel viewport wrt its parent window.
         * 
         * @param pvp the viewport in pixels.
         */
        void setPixelViewport( const eq::PixelViewport& pvp );

        /** 
         * Return this channel's pixel viewport.
         * 
         * @return the pixel viewport.
         */
        const eq::PixelViewport& getPixelViewport() const { return _pvp; }

        /**
         * Notify this channel that the viewport has changed.
         *
         * The channel updates the viewport or pixel viewport accordingly. 
         */
        void notifyViewportChanged();

        /** 
         * Set the channel's viewport wrt its parent window.
         * 
         * @param vp the fractional viewport.
         */
        void setViewport( const eq::Viewport& vp );

        /** 
         * Return this channel's viewport.
         * 
         * @return the fractional viewport.
         */
        const eq::Viewport& getViewport() const { return _vp; }

        /** 
         * Returns the current near and far planes for this channel.
         *
         * @param nearPlane a pointer to store the near plane.
         * @param farPlane a pointer to store the far plane.
         */
        void getNearFar( float* nearPlane, float* farPlane ) const 
            { *nearPlane = _near; *farPlane = _far; }

        /** The last drawing compound for this entity. */
        void setLastDrawCompound( const Compound* compound )
            { _lastDrawCompound = compound; }
        const Compound* getLastDrawCompound() const { return _lastDrawCompound;}
        //*}

        /**
         * @name Operations
         */
        //*{
        /** Update (init and exit) this channel as needed. */
        void updateRunning( const uint32_t initID );

        /** Finalize the last updateRunning changes. */
        bool syncRunning();

        /** 
         * Starts exiting this node.
         */
        void startConfigExit();

        /** 
         * Synchronize the exit of the node.
         * 
         * @return <code>true</code> if the node exited cleanly,
         *         <code>false</code> if not.
         */
        bool syncConfigExit();

        /** 
         * Update one frame.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @param frameNumber the number of the frame.
         * @return true if at least one rendering task was sent.
         */
        bool updateDraw( const uint32_t frameID, const uint32_t frameNumber );

        /** 
         * Trigger the post-draw operations.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @param frameNumber the number of the frame.
         */
        void updatePost( const uint32_t frameID, const uint32_t frameNumber );

        void send( net::ObjectPacket& packet );
        void send( net::ObjectPacket& packet, const std::string& string );
        template< typename T >
        void send( net::ObjectPacket &packet, const std::vector<T>& data );
        //*}

        /** @name Channel listener interface. */
        //*{
        /** Register a channel listener. */
        void addListener( ChannelListener* listener );
        /** Deregister a channel listener. */
        void removeListener( ChannelListener* listener );
        //*}

       /**
         * @name Attributes
         */
        //*{
        void setIAttribute( const eq::Channel::IAttribute attr,
                            const int32_t value )
            { _iAttributes[attr] = value; }
        int32_t  getIAttribute( const eq::Channel::IAttribute attr ) const
            { return _iAttributes[attr]; }
        //*}

        /** @name Error information. */
        //@{
        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        //@}
        
        /* Set the channel's drawable. */
        void setDrawable( const uint32_t drawable );

        /* Get the channel's drawable. */
        uint32_t getDrawable() const { return _drawable; }
        
    protected:
        /** Registers request packets waiting for a return value. */
        base::RequestHandler _requestHandler;

        /** @sa net::Object::attachToSession. */
        virtual void attachToSession( const uint32_t id, 
                                      const uint32_t instanceID, 
                                      net::Session* session );
    private:
        //-------------------- Members --------------------
        /** Number of activations for this channel. */
        uint32_t _active;

        /** The view used by this channel. */
        const View* _view;

        /** The segment used by this channel. */
        const Segment* _segment;

        /** The reason for the last error. */
        std::string _error;

        /** The parent window. */
        Window* _window;
        friend class Window;

        std::string _name;

        /** Integer attributes. */
        int32_t _iAttributes[eq::Channel::IATTR_ALL];

        /** The fractional viewport with respect to the window. */
        eq::Viewport _vp;

        /** The pixel viewport within the window. */
        eq::PixelViewport _pvp;

        /** The alternate drawable (fbo) to use. */
        uint32_t _drawable;
        
        /** true if the pvp is immutable, false if the vp is immutable */
        bool _fixedPVP;

        /** Frustum near plane. */
        float        _near;
        /** Frustum far plane. */
        float        _far;

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** The current state for state change synchronization. */
        base::Monitor< State > _state;

        /** The last draw compound for this entity */
        const Compound* _lastDrawCompound;

        typedef std::vector< ChannelListener* > ChannelListeners;
        ChannelListeners _listeners;

        CHECK_THREAD_DECLARE( _serverThread );

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        //-------------------- Methods --------------------
        /** common code for all constructors */
        void _construct();

        vmml::Vector3ub _getUniqueColor() const;

        void _configInit( const uint32_t initID );
        bool _syncConfigInit();
        void _configExit();
        bool _syncConfigExit();

        void _fireLoadData( const uint32_t frameNumber, 
                            const uint32_t nStatistics,
                            const eq::Statistic* statistics );

        virtual void getInstanceData( net::DataOStream& os ) { EQDONTCALL }
        virtual void applyInstanceData( net::DataIStream& is ) { EQDONTCALL }

        /* command handler functions. */
        net::CommandResult _cmdConfigInitReply( net::Command& command );
        net::CommandResult _cmdConfigExitReply( net::Command& command );
        net::CommandResult _cmdSetNearFar( net::Command& command );
        net::CommandResult _cmdFrameFinishReply( net::Command& command );

        // For access to _fixedPVP
        friend std::ostream& operator << ( std::ostream&, const Channel*);
    };

    std::ostream& operator << ( std::ostream& os, const Channel* channel);
}
}
#endif // EQSERVER_CHANNEL_H
