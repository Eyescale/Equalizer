
#include <test.h>

#include <eq/base/lock.h>
#include <eq/net/command.h>
#include <eq/net/connection.h>
#include <eq/net/init.h>
#include <eq/net/node.h>
#include <eq/net/packets.h>

#include <iostream>

using namespace eqBase;
using namespace eqNet;
using namespace std;

Lock lock;

struct DataPacket : public NodePacket
{
    DataPacket()
        {
            command  = CMD_NODE_CUSTOM;
            size     = sizeof( DataPacket );
            data[0]  = '\0';
        }
    
    char     data[8];
};

class Server : public Node
{
public:
    Server()
        { 
            registerCommand( CMD_NODE_CUSTOM, 
                             CommandFunc<Server>( this, &Server::command ));
        }

protected:
    CommandResult command( Command& cmd )
        {
            TEST( cmd->command == CMD_NODE_CUSTOM );

            const DataPacket* packet = cmd.getPacket<DataPacket>();
            cerr << "Server received: " << packet->data << endl;
            lock.unset();
            return COMMAND_HANDLED;
        }
};

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    lock.set();
    Server server;

    RefPtr<ConnectionDescription> connDesc = new ConnectionDescription;
    
    connDesc->type       = CONNECTIONTYPE_TCPIP;
    connDesc->TCPIP.port = 4242;

    server.addConnectionDescription( connDesc );
    TEST( server.listen( ));

    RefPtr<eqNet::Node> client = new eqNet::Node;
    TEST( client->listen( ));

    RefPtr<Node> serverProxy = new Node;

    connDesc->setHostname( "localhost" );
    serverProxy->addConnectionDescription( connDesc );

    const char message[] = "Don't Panic!";
    DataPacket packet;
    serverProxy->send( packet, message );

    lock.set();
    TEST( serverProxy->getRefCount() == 1 );
    serverProxy = NULL;

    TEST( client->getRefCount() == 1 );
    client      = NULL;
}
