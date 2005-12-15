
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIG_H
#define EQ_CONFIG_H

#include <eq/base/base.h>
#include <eq/base/requestHandler.h>
#include <eq/net/packets.h>
#include <eq/net/session.h>

#include "commands.h"

namespace eq
{
    struct ConfigPacket;
    class Server;

    class Config : public eqNet::Session
    {
    public:
        /** 
         * Constructs a new config.
         * 
         * @param server the server hosting the config.
         */
        Config();

        /** 
         * Initialises this configuration.
         * 
         * @return <code>true</code> if the initialisation was successful,
         *         <code>false</code> if not.
         */
        bool init();

        /** 
         * Exits this configuration.
         * 
         * A config which could not be exited properly may not be
         * re-initialised.
         *
         * @return <code>true</code> if the exit was successful,
         *         <code>false</code> if not.
         */
        bool exit();

    private:
        /** The local proxy of the server hosting the session. */
        friend class Server;
        Server* _server;

        /** Registers pending requests waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** The command functions. */
        void _cmdInitReply( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdExitReply( eqNet::Node* node, const eqNet::Packet* packet );
    };
}

#endif // EQ_CONFIG_H

