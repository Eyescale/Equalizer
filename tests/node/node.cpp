
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

eqBase::Lock lock;

struct DataPacket : public eqNet::NodePacket
{
    DataPacket()
        {
            command  = eqNet::CMD_NODE_CUSTOM;
            size     = sizeof( DataPacket );
            data[0]  = '\0';
        }
    
    char     data[8];
};

class Server : public eqNet::Node
{
public:
    Server() {}

    virtual bool listen()
        {
            if( !eqNet::Node::listen( ))
                return false;

            registerCommand( eqNet::CMD_NODE_CUSTOM, 
                             eqNet::CommandFunc<Server>(this, &Server::command),
                             getCommandThreadQueue( ));
            return true;
        }

protected:
    eqNet::CommandResult command( eqNet::Command& cmd )
        {
            TEST( cmd->command == eqNet::CMD_NODE_CUSTOM );

            const DataPacket* packet = cmd.getPacket< DataPacket >();
            cerr << "Server received: " << packet->data << endl;
            lock.unset();
            return eqNet::COMMAND_HANDLED;
        }
};

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    lock.set();
    eqBase::RefPtr< Server >        server   = new Server;
    eqNet::ConnectionDescriptionPtr connDesc = new eqNet::ConnectionDescription;
    
    connDesc->type       = eqNet::CONNECTIONTYPE_TCPIP;
    connDesc->TCPIP.port = 4242;
    connDesc->setHostname( "localhost" );

    server->addConnectionDescription( connDesc );
    TEST( server->listen( ));

    eqNet::NodePtr client = new eqNet::Node;
    TEST( client->listen( ));

    eqNet::NodePtr serverProxy = new eqNet::Node;

    serverProxy->addConnectionDescription( connDesc );
    TEST( client->connect( serverProxy ));

    const char message[] = "Don't Panic!";
    DataPacket packet;
    serverProxy->send( packet, message );

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
