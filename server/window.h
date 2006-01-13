
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_WINDOW_H
#define EQS_WINDOW_H

#include "pipe.h"

#include <eq/net/base.h>
#include <eq/net/object.h>

#include <iostream>
#include <vector>

namespace eqs
{
    class Channel;
    class Pipe;

    /**
     * The window.
     */
    class Window : public eqNet::Base, public eqNet::Object
    {
    public:
        /** 
         * Constructs a new Window.
         */
        Window();

        virtual ~Window(){}

        /** 
         * Adds a new channel to this window.
         * 
         * @param channel the channel.
         */
        void addChannel( Channel* channel );

        /** 
         * Removes a channel from this window.
         * 
         * @param channel the channel
         * @return <code>true</code> if the channel was removed, <code>false</code>
         *         otherwise.
         */
        bool removeChannel( Channel* channel );

        /** 
         * Returns the number of channels on this window.
         * 
         * @return the number of channels on this window. 
         */
        uint32_t nChannels() const { return _channels.size(); }

        /** 
         * Gets a channel.
         * 
         * @param index the channel's index. 
         * @return the channel.
         */
        Channel* getChannel( const uint32_t index ) const
            { return _channels[index]; }

        Node* getNode() const { return (_pipe ? _pipe->getNode() : NULL); }
        Config* getConfig() const 
            { return (_pipe ? _pipe->getConfig() : NULL); }
        
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
        std::vector<Channel*> _channels;

        /** Number of entitities actively using this window. */
        uint32_t _used;

        /** The parent pipe. */
        Pipe* _pipe;
        friend class Pipe;

        /** The request id for pending asynchronous operations. */
        uint32_t _pendingRequestID;

        void _send( const eqNet::Packet& packet ) { getNode()->send( packet ); }

        void _sendInit();
        void _sendExit();

        void _cmdInitReply(eqNet::Node* node, const eqNet::Packet* packet);
        void _cmdExitReply(eqNet::Node* node, const eqNet::Packet* packet);
    };

    std::ostream& operator << ( std::ostream& os, const Window* window );
};
#endif // EQS_WINDOW_H
