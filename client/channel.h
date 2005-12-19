/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CHANNEL_H
#define EQ_CHANNEL_H

#include "commands.h"
#include "window.h"

#include <eq/net/base.h>
#include <eq/net/object.h>

// namespace eqNet
// {
//     class  Node;
//     struct Packet;
// }

namespace eq
{
    class Channel;
    class Node;

    class Channel : public eqNet::Base, public eqNet::Object
    {
    public:
        /** 
         * Constructs a new channel.
         */
        Channel();

        /**
         * Destructs the channel.
         */
        virtual ~Channel();

        Pipe* getPipe() const { return ( _window ? _window->getPipe() : NULL );}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialises this channel.
         * TODO: Create thread, get display info, etc.
         */
        virtual bool init(){ return true; }

        /** 
         * Exit this channel.
         */
        virtual void exit(){}
        //@}

    private:
        /** The parent node. */
        friend class Window;
        Window*      _window;

        void _pushRequest( eqNet::Node* node, const eqNet::Packet* packet );
        void _reqInit( eqNet::Node* node, const eqNet::Packet* packet );
        void _reqExit( eqNet::Node* node, const eqNet::Packet* packet );
    };
}

#endif // EQ_CHANNEL_H

