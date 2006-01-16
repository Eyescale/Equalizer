
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CHANNEL_H
#define EQS_CHANNEL_H

#include "node.h"
#include "window.h"

#include <eq/commands.h>
#include <eq/base/viewport.h>
#include <eq/net/base.h>
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
    class Channel : public eqNet::Base, public eqNet::Object
    {
    public:
        /** 
         * Constructs a new Channel.
         */
        Channel();

        /**
         * @name Data Access
         */
        //*{
        Node* getNode() const { return (_window ? _window->getNode() : NULL); }
        Config* getConfig() const 
            { return (_window ? _window->getConfig() : NULL); }

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

        /** 
         * Return the 
         * 
         * 
         * @return 
         */
        const eqBase::PixelViewport& getPixelViewport() const { return _pvp; }
        //*/

        /**
         * @name Operations
         */
        //*{
        /** 
         * Start initializing this node.
         */
        void startInit();

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
         */
        void update();

        /** 
         * Send a command packet to the channel.
         * 
         * @param packet the packet.
         */
        void send( eqNet::Packet& packet ) { getNode()->send( packet ); }
        //*}


    private:
        /** Number of entitities actively using this channel. */
        uint32_t _used;

        /** The parent window. */
        Window* _window;
        friend class Window;

        /** The request identifier for pending asynchronous operations. */
        uint32_t _pendingRequestID;

        /** The fractional viewport with respect to the window. */
        eqBase::Viewport      _vp;

        /** The pixel viewport within the window. */
        eqBase::PixelViewport _pvp;

        void _sendInit();
        void _sendExit();

        void _cmdInitReply(eqNet::Node* node, const eqNet::Packet* packet);
        void _cmdExitReply(eqNet::Node* node, const eqNet::Packet* packet);
    };

    std::ostream& operator << ( std::ostream& os, const Channel* channel);
};
#endif // EQS_CHANNEL_H
