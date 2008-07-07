
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>

#include <eq/base/lock.h>
#include <eq/net/command.h>
#include <eq/net/connection.h>
#include <eq/net/init.h>
#include <eq/net/node.h>
#include <eq/net/packets.h>

#include <iostream>

using namespace std;

namespace
{
eq::base::Lock lock;
static const string message =
    "Don't Panic! And now some more text to make the message bigger";
#define NMESSAGES 1000
}

struct DataPacket : public eq::net::NodePacket
{
    DataPacket()
        {
            command  = eq::net::CMD_NODE_CUSTOM;
            size     = sizeof( DataPacket );
            data[0]  = '\0';
        }
    
    char     data[8];
};

class Server : public eq::net::Node
{
public:
    Server() : _messagesLeft( NMESSAGES ){}

    virtual bool listen()
        {
            if( !eq::net::Node::listen( ))
                return false;

            registerCommand( eq::net::CMD_NODE_CUSTOM, 
                             eq::net::CommandFunc<Server>(this, &Server::command),
                             getCommandThreadQueue( ));
            return true;
        }

protected:
    eq::net::CommandResult command( eq::net::Command& cmd )
        {
            TEST( cmd->command == eq::net::CMD_NODE_CUSTOM );
            TEST( _messagesLeft > 0 );

            const DataPacket* packet = cmd.getPacket< DataPacket >();
            TESTINFO( message == packet->data, packet->data );

            --_messagesLeft;
            if( !_messagesLeft )
                lock.unset();

            return eq::net::COMMAND_HANDLED;
        }

private:
    unsigned _messagesLeft;
};

int main( int argc, char **argv )
{
    eq::net::init( argc, argv );

    lock.set();
    eq::base::RefPtr< Server >        server   = new Server;
    eq::net::ConnectionDescriptionPtr connDesc = new eq::net::ConnectionDescription;
    
    connDesc->type       = eq::net::CONNECTIONTYPE_TCPIP;
    connDesc->TCPIP.port = 4242;
    connDesc->setHostname( "localhost" );

    server->addConnectionDescription( connDesc );
    TEST( server->listen( ));

    eq::net::NodePtr client = new eq::net::Node;
    TEST( client->listen( ));

    eq::net::NodePtr serverProxy = new eq::net::Node;

    serverProxy->addConnectionDescription( connDesc );
    TEST( client->connect( serverProxy ));

    DataPacket packet;

    eq::base::Clock clock;
    for( unsigned i = 0; i < NMESSAGES; ++i )
        serverProxy->send( packet, message );
    const float time = clock.getTimef();

    const size_t size = NMESSAGES * ( packet.size + message.length() - 7 );
    cout << "Send " << size << " bytes using " << NMESSAGES << " packets in "
         << time << "ms" << " (" << size / 1024. * 1000.f / time << " KB/s)" 
         << endl;

    lock.set();

    TEST( client->disconnect( serverProxy ));
    TESTINFO( serverProxy->getRefCount() == 1, serverProxy->getRefCount( ));
    serverProxy = 0;

    TEST( client->stopListening( ));
    TESTINFO( client->getRefCount() == 1, client->getRefCount( ));
    client      = 0;

    TEST( server->stopListening( ));
    TESTINFO( server->getRefCount() == 1, server->getRefCount( ));
    server      = 0;
}
