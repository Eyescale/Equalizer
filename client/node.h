
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODE_H
#define EQ_NODE_H

#include "commands.h"

#include <eq/net/node.h>

namespace eq
{
    class Node : public eqNet::Node
    {
    public:
        /** 
         * Constructs a new node.
         */
        Node();

        /**
         * Destructs the node.
         */
        virtual ~Node();

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialises this node.
         */
        virtual void init(){};

        /** 
         * Exit this node.
         */
        virtual void exit(){};

        //@}

    protected:
        /** 
         * @sa eqNet::Node::handlePacket
         */
        virtual void handlePacket( eqNet::Node* node, 
                                   const eqNet::Packet* packet );

    private:
        void _handleCommand( const eqNet::Packet* packet );

        /** The command handler function table. */
        void (eq::Node::*_cmdHandler[CMD_NODE_ALL])
            ( const eqNet::Packet* packet );

        void _cmdUnknown( const eqNet::Packet* packet );
        void _cmdInit( const eqNet::Packet* packet ) { init(); }
        //void _cmdChooseConfigReply( const eqNet::Packet* packet );
    };
}

#endif // EQ_NODE_H

