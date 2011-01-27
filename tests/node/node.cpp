
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <test.h>

#include <co/base/monitor.h>
#include <co/command.h>
#include <co/connection.h>
#include <co/connectionDescription.h>
#include <co/init.h>
#include <co/node.h>
#include <co/packets.h>

#include <iostream>

using namespace std;

namespace
{

co::base::Monitor<bool> monitor( false ); 

static const string message =
    "Don't Panic! And now some more text to make the message bigger";
#define NMESSAGES 1000
}

struct DataPacket : public co::NodePacket
{
    DataPacket()
        {
            command  = co::CMD_NODE_CUSTOM;
            size     = sizeof( DataPacket );
            data[0]  = '\0';
        }
    
    char     data[8];
};

class Server : public co::LocalNode
{
public:
    Server() : _messagesLeft( NMESSAGES ){}

    virtual bool listen()
        {
            if( !co::LocalNode::listen( ))
                return false;

            registerCommand( co::CMD_NODE_CUSTOM, 
                             co::CommandFunc<Server>(
                                                       this, &Server::command),
                             getCommandThreadQueue( ));
            return true;
        }

protected:
    bool command( co::Command& cmd )
        {
            TEST( cmd->command == co::CMD_NODE_CUSTOM );
            TEST( _messagesLeft > 0 );

            const DataPacket* packet = cmd.getPacket< DataPacket >();
            TESTINFO( message == packet->data, packet->data );

            --_messagesLeft;
            if( !_messagesLeft )
                monitor.set( true );

            return true;
        }

private:
    unsigned _messagesLeft;
};

int main( int argc, char **argv )
{
    co::init( argc, argv );

    co::base::RefPtr< Server >        server   = new Server;
    co::ConnectionDescriptionPtr connDesc = 
        new co::ConnectionDescription;
    
    connDesc->type = co::CONNECTIONTYPE_TCPIP;
    connDesc->port = 4242;
    connDesc->setHostname( "localhost" );

    server->addConnectionDescription( connDesc );
    TEST( server->listen( ));

    co::NodePtr serverProxy = new co::Node;
    serverProxy->addConnectionDescription( connDesc );

    connDesc = new co::ConnectionDescription;
    connDesc->type       = co::CONNECTIONTYPE_TCPIP;
    connDesc->setHostname( "localhost" );

    co::LocalNodePtr client = new co::LocalNode;
    client->addConnectionDescription( connDesc );
    TEST( client->listen( ));
    TEST( client->connect( serverProxy ));

    DataPacket packet;

    co::base::Clock clock;
    for( unsigned i = 0; i < NMESSAGES; ++i )
        serverProxy->send( packet, message );
    const float time = clock.getTimef();

    const size_t size = NMESSAGES * ( packet.size + message.length() - 7 );
    cout << "Send " << size << " bytes using " << NMESSAGES << " packets in "
         << time << "ms" << " (" << size / 1024. * 1000.f / time << " KB/s)" 
         << endl;

    monitor.waitEQ( true );

    TEST( client->disconnect( serverProxy ));
    TEST( client->close( ));
    TEST( server->close( ));

    TESTINFO( serverProxy->getRefCount() == 1, serverProxy->getRefCount( ));
    TESTINFO( client->getRefCount() == 1, client->getRefCount( ));
    TESTINFO( server->getRefCount() == 1, server->getRefCount( ));

    serverProxy = 0;
    client      = 0;
    server      = 0;

    return EXIT_SUCCESS;
}
