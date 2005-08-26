
#include <eq/base/lock.h>
#include <eq/net/connection.h>
#include <eq/net/node.h>
#include <eq/net/packet.h>
#include <eq/net/global.h>

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
    
    uint nBytes;
};

class Server : public Node
{
protected:
    virtual void handleCommand( Node* node, const NodePacket* pkg )
        {
            if( pkg->command != CMD_NODE_CUSTOM )
                return;

            DataPacket* packet = (DataPacket*)pkg;

            uint64 nBytes = packet->nBytes;
            char*  data   = (char*)alloca( nBytes );

            if( !node->recv( data, nBytes ))
                exit( EXIT_FAILURE );

            ASSERT( data[nBytes-1] == '\0' );
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
            toNode->send( packet );
            toNode->send( string, packet.nBytes );
        }
};

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    lock.set();
    Server server;

    Connection *connection = Connection::create(TYPE_TCPIP);
    ConnectionDescription connDesc;
    //sprintf( connDesc.hostname, "benjy" );
    connDesc.parameters.TCPIP.port = 4242;

    if( !connection->listen( connDesc ))
        exit( EXIT_FAILURE );
    if( !server.listen( connection ))
        exit( EXIT_FAILURE );
    
    connection = Connection::create(TYPE_TCPIP);
    sprintf( connDesc.hostname, "localhost" );
    if( !connection->connect( connDesc ))
        exit( EXIT_FAILURE );

    Client client;
    if( !client.listen( connection ))
        exit( EXIT_FAILURE );

    Node serverProxy;
    if( !client.connect( &serverProxy, connection ))
        exit( EXIT_FAILURE );

    const char message[] = "Don't Panic!";
    client.send( &serverProxy, message );

    lock.set();
}

