
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CLIENT_H
#define EQ_CLIENT_H

#include "commands.h"

#include <eq/base/monitor.h>
#include <eq/net/node.h>

namespace eq
{
    class Client : public eqNet::Node
    {
    public:
        /** 
         * Constructs a new client.
         */
        Client();

        /**
         * Destructs the client.
         */
        virtual ~Client();

        /** @name Referenced by node threads. */
        //*{
        void refUsed() { ++_used; }
        void unrefUsed() { --_used; }
        //*}

    protected:
        /** @sa eqNet::Node::clientLoop */
        virtual void clientLoop();

        /** 
         * @sa eqNet::Node::handleCommand
         */
        virtual eqNet::CommandResult handleCommand( eqNet::Command& command );

        /** @sa eqNet::Node::createNode */
        virtual eqBase::RefPtr<eqNet::Node> createNode( const CreateReason
                                                        reason );
        
        /** @sa eqNet::Node::createSession */
        virtual eqNet::Session* createSession();

    private:
        eqBase::Monitor<uint32_t> _used;
    };
}

#endif // EQ_CLIENT_H
