
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CHANNEL_H
#define EQS_CHANNEL_H

#include "node.h"
#include "window.h"

#include <eq/commands.h>
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

        Node* getNode() const { return (_window ? _window->getNode() : NULL); }

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
         * Send the node the command to stop its execution.
         */
        void stop();
        //*}


    private:
        /** Number of entitities actively using this channel. */
        uint _used;

        /** The parent window. */
        Window* _window;
        friend class Window;

        /** The request identifier for pending asynchronous operations. */
        uint _pendingRequestID;

        void _send( eqNet::Packet& packet ) { getNode()->send( packet ); }
        void _sendInit();
        void _sendExit();

        void _cmdInitReply(eqNet::Node* node, const eqNet::Packet* packet);
        void _cmdExitReply(eqNet::Node* node, const eqNet::Packet* packet);
    };

    std::ostream& operator << ( std::ostream& os, const Channel* channel);
};
#endif // EQS_CHANNEL_H
