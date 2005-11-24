
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODE_H
#define EQ_NODE_H

#include "commands.h"

#include <eq/net/node.h>

namespace eq
{
    class Config;
    class Server;

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
        virtual bool init(){ return true; }

        /** 
         * Exit this node.
         */
        virtual void exit(){}
        //@}

        uint getID() const { return _id; }
        Config* getConfig() const { return _config; }

    protected:
        /** 
         * @sa eqNet::Node::handlePacket
         */
        virtual void handlePacket( eqNet::Node* node, 
                                   const eqNet::Packet* packet );

        /** 
         * @sa eqNet::Node::createNode
         */
        virtual eqBase::RefPtr<eqNet::Node> createNode();

    private:
        uint                   _id;
        eqBase::RefPtr<Server> _server;
        Config*                _config;

        /** The command handler function table. */
        void (eq::Node::*_cmdHandler[CMD_NODE_ALL])
            ( eqNet::Node* node, const eqNet::Packet* packet );

        void _cmdInit( eqNet::Node* node, const eqNet::Packet* packet );
        //void _cmdChooseConfigReply( const eqNet::Packet* packet );
    };
}

#endif // EQ_NODE_H

