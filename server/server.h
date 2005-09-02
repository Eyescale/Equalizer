
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_SERVER_H
#define EQS_SERVER_H

#include <eq/net/node.h>

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
