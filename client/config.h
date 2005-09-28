
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIG_H
#define EQ_CONFIG_H

#include <eq/base/base.h>
#include <eq/base/requestHandler.h>
#include <eq/net/packet.h>

#include "commands.h"

namespace eq
{
    class Server;

    class Config
    {
    public:
        /** 
         * Constructs a new config.
         * 
         * @param id the server-supplied identifier of the config.
         * @param server the server hosting the session.
         */
        Config( const uint id, Server* server );

        /** 
         * Returns the identifier of this config.
         * 
         * @return the identifier of this config.
         */
        uint getID() const { return _id; }

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
        /** The identifier of this config. */
        const uint _id;

        /** The local proxy of the server hosting the session. */
        Server* _server;

        /** Registers pending requests waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** The command handler function table. */
        void (eq::Server::*_cmdHandler[CMD_CONFIG_ALL])
            ( const eqNet::Packet* packet );

        void _cmdUnknown( const eqNet::Packet* packet );
        //void _cmd( const eqNet::Packet* packet );
    };
}

#endif // EQ_CONFIG_H

