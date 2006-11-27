
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CHANNEL_H
#define EQS_CHANNEL_H

#include "node.h"
#include "window.h"

#include <eq/client/commands.h>
#include <eq/client/object.h>
#include <eq/client/viewport.h>
#include <eq/net/packets.h>

#include <iostream>
#include <vector>

namespace eqs
{
    class Window;

    /**
     * The channel.
     */
    class Channel : public eqNet::Object
    {
    public:
        enum State
        {
            STATE_STOPPED,       // initial     <----+
            STATE_INITIALIZING,  // init sent        |
            STATE_RUNNING,       // init successful  |
            STATE_STOPPING       // exit send   -----+
        };

        /** 
         * Constructs a new Channel.
         */
        Channel();

        /** 
         * Constructs a new deep copy of a channel.
         */
        Channel( const Channel& from );

        /** 
         * @return the state of this pipe.
         */
        State getState()    const { return _state; }

        /**
         * @name Data Access
         */
        //*{
        Node* getNode() const { return (_window ? _window->getNode() : NULL); }
        Window* getWindow() const { return _window; }
        Config* getConfig() const { return _window ? _window->getConfig():NULL;}

        /** 
         * References this window as being actively used.
         */
        void refUsed();

        /** 
         * Unreferences this window as being actively used.
         */
        void unrefUsed();

        /** 
         * Returns if this window is actively used.
         *
         * @return <code>true</code> if this window is actively used,
         *         <code>false</code> if not.
         */
        bool isUsed() const { return (_used!=0); }

        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        /** 
         * Set the channel's pixel viewport wrt its parent pipe.
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
         * Notify this channel that it's parent window pixel viewport has
         * changed. 
         *
         * The channel updates the viewport or pixel viewport accordingly. 
         */
        void notifyWindowPVPChanged();

        /** 
         * Set the channel's viewport wrt its parent pipe.
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
         * @param near a pointer to store the near plane.
         * @param far a pointer to store the far plane.
         */
        void getNearFar( float* near, float* far ) const 
            { *near = _near; *far = _far; }
        //*}

        /**
         * @name Operations
         */
        //*{
        /** 
         * Start initializing this node.
         *
         * @param initID an identifier to be passed to all init methods.
         */
        void startInit( const uint32_t initID );

        /** 
         * Synchronize the initialisation of the node.
         * 
         * @return <code>true</code> if the node was initialised successfully,
         *         <code>false</code> if not.
         */
        bool syncInit();
        
        /** 
         * Starts exiting this node.
         */
        void startExit();

        /** 
         * Synchronize the exit of the node.
         * 
         * @return <code>true</code> if the node exited cleanly,
         *         <code>false</code> if not.
         */
        bool syncExit();

        /** 
         * Update the per-frame data of this channel.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         */
        void update( const uint32_t frameID );

        /** @sa eqNet::Object::send */
        bool send( eqNet::ObjectPacket& packet )
            { return eqNet::Object::send( _getNetNode(), packet ); }
        //*}

    protected:
        virtual ~Channel();

    private:
        /** The current operational state. */
        State _state;

        /** Number of entitities actively using this channel. */
        uint32_t _used;

        /** The parent window. */
        Window* _window;
        friend class Window;

        std::string _name;

        /** The fractional viewport with respect to the window. */
        eq::Viewport      _vp;

        /** The pixel viewport within the window. */
        eq::PixelViewport _pvp;

        /** 
         * true if the pixel viewport is immutable, false if the viewport is
         * immutable
         */
        bool _fixedPVP;

        /** Frustum near plane. */
        float        _near;
        /** Frustum far plane. */
        float        _far;

        /** The request identifier for pending asynchronous operations. */
        uint32_t _pendingRequestID;

        /** common code for all constructors */
        void _construct();

        void _sendInit( const uint32_t initID );
        void _sendExit();

        eqBase::RefPtr<eqNet::Node> _getNetNode() const 
            { return getNode()->getNode(); }

        /* command handler functions. */
        eqNet::CommandResult _cmdInitReply( eqNet::Command& command );
        eqNet::CommandResult _cmdExitReply( eqNet::Command& command );
        eqNet::CommandResult _reqSetNearFar( eqNet::Command& command );
    };

    std::ostream& operator << ( std::ostream& os, const Channel* channel);
};
#endif // EQS_CHANNEL_H
