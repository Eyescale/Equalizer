
#include <test.h>

#include <eq/base/lock.h>
#include <eq/net/command.h>
#include <eq/net/connection.h>
#include <eq/net/init.h>
#include <eq/net/node.h>
#include <eq/net/packets.h>

#include <alloca.h>
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
                             PacketFunc<Server>( this, &Server::command ));
        }

protected:
    CommandResult command( Command& command )
        {
            TEST( command->command == CMD_NODE_CUSTOM );

            const DataPacket* packet = command.getPacket<DataPacket>();
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

    RefPtr<Connection>            connection = 
        Connection::create( Connection::TYPE_TCPIP );
    RefPtr<ConnectionDescription> connDesc   = connection->getDescription();

    connDesc->TCPIP.port = 4242;

    TEST( connection->listen( ));
    TEST( server.listen( connection ));

    RefPtr<eqNet::Node> client = new eqNet::Node;
    TEST( client->listen( ));

    RefPtr<Node> serverProxy = new Node;

    connDesc->hostname = "localhost";
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
