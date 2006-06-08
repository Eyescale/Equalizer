
#include <test.h>

#include <eq/base/lock.h>
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
        }
    
    uint32_t nBytes;
};

class Server : public Node
{
public:
    Server() : Node( CMD_NODE_CUSTOM+1 ) 
        { 
            registerCommand( CMD_NODE_CUSTOM, this, 
                             reinterpret_cast<CommandFcn>( &Server::command ));
        }

protected:
    void command( Node* node, const NodePacket* pkg )
        {
            TEST( pkg->command == CMD_NODE_CUSTOM );

            DataPacket* packet = (DataPacket*)pkg;

            uint64_t nBytes = packet->nBytes;
            char*    data   = (char*)alloca( nBytes );

            if( !node->recv( data, nBytes ))
                exit( EXIT_FAILURE );

            EQASSERT( data[nBytes-1] == '\0' );
            cerr << "Server received: " << data << endl;
            lock.unset();
        }
};

class Client : public Node
{
public:
    void send( Node* toNode, const char* string )
        {
            DataPacket packet;
            packet.nBytes = strlen(string)+1;
            TEST( toNode->send( packet, string ));
        }
};

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    lock.set();
    Server server;

    RefPtr<Connection> connection = Connection::create(Connection::TYPE_TCPIP);
    RefPtr<ConnectionDescription> connDesc = new ConnectionDescription;
    connDesc->type = Connection::TYPE_TCPIP;
    //connDesc->hostname = "benjy";
    connDesc->TCPIP.port = 4242;

    TEST( connection->listen( connDesc ));
    TEST( server.listen( connection ));

    Client client;
    TEST( client.listen( ));

    Node serverProxy;

    connDesc->hostname = "localhost";
    serverProxy.addConnectionDescription( connDesc );

    const char message[] = "Don't Panic!";
    client.send( &serverProxy, message );

    lock.set();
}

