
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_SERVER_H
#define EQ_SERVER_H

#include <eq/net/node.h>

#include "commands.h"

namespace eq
{
    class Config;
    class ConfigParams;
    class Node;

    class Server : public eqNet::Node
    {
    public:
        /** 
         * Constructs a new server.
         */
        Server();

        /**
         * Destructs this server.
         */
        virtual ~Server(){}

        /** 
         * Opens the connection to an Equalizer server.
         * 
         * @param address the server's address.
         * @return <code>true</code> if the server was opened correctly,
         *         <code>false</code> otherwise.
         */
        bool open( const std::string& address );

        /** 
         * Closes the connection to the server.
         * 
         * @return <code>true</code> if the server was closed correctly,
         *         <code>false</code> otherwise.
         */
        bool close();

        /** 
         * Chooses a configuration on the server.
         * 
         * @param parameters the configuration parameters
         * @return The choosen config, or <code>NULL</code> if no matching
         *         config was found.
         * @sa ConfigParams
         */
        Config* chooseConfig( const ConfigParams* parameters );

        /** 
         * Releases the configuration.
         * 
         * The passed configuration will be destroyed by this function and is no
         * longer valid after the call.
         *
         * @param config the configuration.
         */
        void    releaseConfig( Config* config );

        /** 
         * Handles a packet received for this server.
         * 
         * @param packet the packet.
         */
        void handlePacket( const eqNet::Packet* packet );

        /** 
         * Handles a command for this server.
         * 
         * @param packet the command packet.
         */
        void handleCommand( const eqNet::NodePacket* packet );

    private:
        enum State 
        {
            STATE_STOPPED,
            STATE_OPENED
        };
        State _state;

        /** The allocated configurations, mapped by identifier. */
        eqNet::IDHash<Config*> _configs;

        /** The command handler function table. */
        void (eq::Server::*_cmdHandler[CMD_SERVER_ALL-eqNet::CMD_NODE_CUSTOM])
            ( const eqNet::Packet* packet );

        void _cmdUnknown( const eqNet::Packet* packet );
        void _cmdChooseConfigReply( const eqNet::Packet* packet );
        
        //friend class eq::Node;
    };
}

#endif // EQ_SERVER_H

