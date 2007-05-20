
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CHANNEL_H
#define EQS_CHANNEL_H

#include "node.h"
#include "window.h"

#include <eq/client/commands.h>
#include <eq/client/viewport.h>
#include <eq/net/object.h>
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
        Pipe* getPipe() const { return (_window ? _window->getPipe() : NULL); }
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
         * Notify this channel that the viewport has changed.
         *
         * The channel updates the viewport or pixel viewport accordingly. 
         */
        void notifyViewportChanged();

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
        void getNearFar( float* nearPlane, float* farPlane ) const 
            { *nearPlane = _near; *farPlane = _far; }
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
        void startConfigInit( const uint32_t initID );

        /** 
         * Synchronize the initialisation of the node.
         * 
         * @return <code>true</code> if the node was initialised successfully,
         *         <code>false</code> if not.
         */
        bool syncConfigInit();
        
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
         */
        void updateDraw( const uint32_t frameID, const uint32_t frameNumber );

        /** 
         * Trigger the post-draw operations.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @param frameNumber the number of the frame.
         */
        void updatePost( const uint32_t frameID, const uint32_t frameNumber );

        void send( eqNet::ObjectPacket& packet ) 
            { packet.objectID = getID(); getNode()->send( packet ); }
        void send( eqNet::ObjectPacket& packet, const std::string& string ) 
            { packet.objectID = getID(); getNode()->send( packet, string ); }
        template< typename T >
        void send( eqNet::ObjectPacket &packet, const std::vector<T>& data )
            { packet.objectID = getID(); getNode()->send( packet, data ); }
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
        
    protected:
        virtual ~Channel();

        /** Registers request packets waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

    private:
        /** The current operational state. */
        State _state;

        /** Number of entitities actively using this channel. */
        uint32_t _used;

        /** The reason for the last error. */
        std::string            _error;

        /** The parent window. */
        Window* _window;
        friend class Window;

        std::string _name;

        /** Integer attributes. */
        int32_t _iAttributes[eq::Channel::IATTR_ALL];

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

        vmml::Vector3ub _getUniqueColor() const;

        void _sendConfigInit( const uint32_t initID );
        void _sendConfigExit();

        /* command handler functions. */
        eqNet::CommandResult _cmdConfigInitReply( eqNet::Command& command );
        eqNet::CommandResult _cmdConfigExitReply( eqNet::Command& command );
        eqNet::CommandResult _reqSetNearFar( eqNet::Command& command );

        // For access to _fixedPVP
        friend std::ostream& operator << ( std::ostream&, const Channel*);
    };

    std::ostream& operator << ( std::ostream& os, const Channel* channel);
};
#endif // EQS_CHANNEL_H
