
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
    class OpenParams;
    struct ServerPacket;

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
         * @param params parameters for the server connection.
         * @return <code>true</code> if the server was opened correctly,
         *         <code>false</code> otherwise.
         * @sa OpenParams
         */
        bool open( const OpenParams& params );

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
        Config* chooseConfig( const ConfigParams& parameters );

        /** 
         * Releases the configuration.
         * 
         * The passed configuration will be destroyed by this function and is no
         * longer valid after the call.
         *
         * @param config the configuration.
         */
        void    releaseConfig( Config* config );

        void addConfig( Config* config );
    private:
        enum State 
        {
            STATE_STOPPED,
            STATE_OPENED
        };
        State _state;

        /** The allocated configurations, mapped by identifier. */
        eqNet::IDHash<Config*> _configs;

        /** The command functions. */
        void _cmdUnknown( eqNet::Node* node,  const eqNet::Packet* packet );
        void _cmdChooseConfigReply( eqNet::Node* node, 
                                    const eqNet::Packet* packet );
    };

    inline std::ostream& operator << ( std::ostream& os, const Server* server )
    {
        if( !server )
        {
            os << "NULL server";
            return os;
        }

        os << "server " << (void*)server;
        return os;
    }
}

#endif // EQ_SERVER_H

