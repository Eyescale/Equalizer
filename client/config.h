
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
         * @param id the server-supplied identifier of the config.
         * @param server the server hosting the session.
         */
        Config( const uint id, Server* server );

        /** 
         * Maps the configs' session.
         * 
         * @return <code>true</code> if the config was mapped,
         *         <code>false</code> otherwise.
         */
        bool map();

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

        /** 
         * Handles a command for this config.
         * 
         * @param packet the command packet.
         */
        void handleCommand( const ConfigPacket* packet );

    private:
        /** The local proxy of the server hosting the session. */
        Server* _server;

        /** Registers pending requests waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** The command handler function table. */
        void (eq::Config::*_cmdHandler[CMD_CONFIG_ALL])
            ( const ConfigPacket* packet );

        void _cmdUnknown( const ConfigPacket* packet );
        void _cmdInitReply( const ConfigPacket* packet );
    };
}

#endif // EQ_CONFIG_H

