
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_SERVER_H
#define EQS_SERVER_H

#include <eq/net/node.h>

#include "packets.h"

/** 
 * @namespace eqs
 * @brief The Equalizer server library.
 *
 * This namespace implements the server-side functionality for the Equalizer
 * framework.
 */
namespace eqs
{
    /**
     * The Equalizer server.
     */
    class Server : public eqNet::Node
    {
    public:
        /** 
         * Constructs a new Server.
         */
        Server();

        /** 
         * Runs the server.
         * 
         * @param argc the number of command line arguments.
         * @param argv the command line arguments.
         * @return <code>true</code> if the server did run successfully,
         *         <code>false</code> if not.
         */
        virtual bool run( int argc, char **argv );

    protected:
        /** 
         * @sa eqNet::Node::handlePacket
         */
        virtual void handlePacket( eqNet::Node* node, 
                                   const eqNet::Packet* packet );

        /** 
         * @sa eqNet::Node::handleCommand
         */
        virtual void handleCommand( eqNet::Node* node,
                                    const eqNet::NodePacket* packet );


    private:

        /** The command handler function table. */
        void (eqs::Server::*_cmdHandler[CMD_SERVER_ALL -eqNet::CMD_NODE_CUSTOM])
            ( eqNet::Node* node, const eqNet::Packet* packet );

        void _cmdChooseConfig( eqNet::Node* node,
                               const eqNet::Packet* packet );
    };

    inline std::ostream& operator << ( std::ostream& os, const Server* server )
    {
        if( server )
            os << "server " << (eqNet::Node*)server;
        else
            os << "NULL server";
        
        return os;
    }
};
#endif // EQS_SERVER_H
