
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_SERVER_H
#define EQ_SERVER_H

#include <eq/net/node.h>

namespace eq
{
    class Config;
    class ConfigParams;

    class Server : protected eqNet::Node
    {
    public:
        /** 
         * Constructs a new server.
         */
        Server();

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
         * @param config the configuration.
         */
        void    releaseConfig( Config* config );

    private:
        enum State {
            STATE_STOPPED,
            STATE_OPENED
        };
        State _state;


        /** The command handler function table. */
//         void (eq::Server::*_cmdHandler[CMD_SERVER_ALL-CMD_NODE_CUSTOM])
//             ( Node* node, const Packet* packet );

//         void _cmdChooseConfig( Node* node, const Packet* packet );
    };
}

#endif // EQ_SERVER_H

