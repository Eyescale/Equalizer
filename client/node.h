
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODE_H
#define EQ_NODE_H

#include "commands.h"

#include <eq/net/node.h>
#include <eq/net/object.h>

namespace eq
{
    class Config;
    class Pipe;
    class Server;

    class Node : public eqNet::Node, public eqNet::Object
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
         * Returns the config of this node.
         * 
         * @return the config of this node. 
         */
        Config* getConfig() const { return _config; }

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
        eqBase::RefPtr<Server> _server;
        Config*                _config;
        std::vector<Pipe*>     _pipes;

        void _addPipe( Pipe* pipe );
        void _removePipe( Pipe* pipe );

        /** The command handler function table. */
        void (eq::Node::*_cmdHandler[CMD_NODE_ALL])
            ( eqNet::Node* node, const eqNet::Packet* packet );

        void _cmdCreateConfig( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdCreatePipe( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdDestroyPipe( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdInit( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdExit( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdStop( eqNet::Node* node, const eqNet::Packet* packet );
    };
}

#endif // EQ_NODE_H

