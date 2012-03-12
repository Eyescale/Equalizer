
/* Copyright (c) 2012, Stefan Eilemann <eile@eyescale.ch> 
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

#include <co/base/clock.h>
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

#define NMESSAGES 1000
}

struct DataPacket : public co::NodePacket
{
    DataPacket()
            : sleepTime( 0 )
        {
            command  = co::CMD_NODE_CUSTOM;
            size     = sizeof( DataPacket );
        }
    
    uint32_t sleepTime;
    uint8_t data[EQ_32KB];
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
                             co::CommandFunc<Server>( this, &Server::command ),
                             0 );
            return true;
        }

protected:
    bool command( co::Command& cmd )
        {
            cout << "Buh" <<std::endl;
            TEST( cmd->command == co::CMD_NODE_CUSTOM );
            TEST( _messagesLeft > 0 );

            const DataPacket* packet = cmd.get< DataPacket >();
            if( packet->sleepTime > 0 )
                co::base::sleep( packet->sleepTime );

            --_messagesLeft;
            if( !_messagesLeft )
            {
                const float time = _clock.resetTimef();
                const size_t size = NMESSAGES * packet->size;
                cout << "Got " << size << " bytes using " << NMESSAGES
                     << " packets in " << time << "ms" << " (" 
                     << size / 1024. * 1000.f / time << " KB/s)" << endl;

                monitor.set( !monitor.get( ));
                _messagesLeft = NMESSAGES;
            }
            return true;
        }

private:
    unsigned _messagesLeft;
    co::base::Clock _clock;
};

class Client : public co::LocalNode
{
public:
    virtual bool listen()
        {
            if( !co::LocalNode::listen( ))
                return false;

            registerCommand( co::CMD_NODE_CUSTOM,
                             co::CommandFunc<Client>( this, &Client::command ),
                             0 );
            return true;
        }
protected:
    bool command( co::Command& cmd ) { return true; }
};

int main( int argc, char **argv )
{
#ifdef Linux
    return true; // localhost multicast not working on my Linux machine
#endif

    co::init( argc, argv );

    co::base::RefPtr< Server > server = new Server;
    co::ConnectionDescriptionPtr tcp = new co::ConnectionDescription;
    co::ConnectionDescriptionPtr rsp = new co::ConnectionDescription;
    
    tcp->type = co::CONNECTIONTYPE_TCPIP;
    tcp->port = 4242;
    tcp->setHostname( "localhost" );
    rsp->type = co::CONNECTIONTYPE_RSP;
    rsp->port = 4242;
    rsp->setInterface( "127.0.0.1" );

    server->addConnectionDescription( tcp );
    server->addConnectionDescription( rsp );
    TEST( server->listen( ));

    co::NodePtr serverProxy = new co::Node;
    serverProxy->addConnectionDescription( tcp );
    serverProxy->addConnectionDescription( rsp );

    tcp = new co::ConnectionDescription;
    rsp = new co::ConnectionDescription;

    tcp->type = co::CONNECTIONTYPE_TCPIP;
    tcp->setHostname( "localhost" );
    rsp->type = co::CONNECTIONTYPE_RSP;
    rsp->port = 4242;
    rsp->setInterface( "127.0.0.1" );

    co::LocalNodePtr client = new Client;
    client->addConnectionDescription( tcp );
    client->addConnectionDescription( rsp );
    TEST( client->listen( ));
    TEST( client->connect( serverProxy ));

    DataPacket packet;

    co::base::Clock clock;
    for( unsigned i = 0; i < NMESSAGES; ++i )
    {
        TEST( serverProxy->multicast( packet ));
    }
    const float time = clock.getTimef();
    const size_t size = NMESSAGES * ( packet.size );
    cout << "Send " << size << " bytes using " << NMESSAGES << " packets in "
         << time << "ms" << " (" << size / 1024. * 1000.f / time << " KB/s)" 
         << endl;

    monitor.waitEQ( true );

    // repro for https://github.com/Eyescale/Equalizer/issues/96
    packet.sleepTime = 10; //ms
    for( unsigned i = 0; i < NMESSAGES; ++i )
        TEST( serverProxy->multicast( packet ));
    monitor.waitEQ( false );

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
