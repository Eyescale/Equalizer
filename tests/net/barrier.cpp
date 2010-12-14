
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <pthread.h> // must come first!

#include <test.h>

#include <eq/net/barrier.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/init.h>
#include <eq/net/node.h>

#include <iostream>

eq::base::Monitor< eq::net::Barrier* > _barrier( 0 );

class NodeThread : public eq::base::Thread
{
public:
    NodeThread( const bool master ) : _master(master) {}

    virtual void run()
        {
            eq::net::ConnectionDescriptionPtr description = 
                new eq::net::ConnectionDescription;
            description->type = eq::net::CONNECTIONTYPE_TCPIP;
            description->port = _master ? 4242 : 4243;

            eq::net::LocalNodePtr node = new eq::net::LocalNode;
            node->addConnectionDescription( description );
            TEST( node->listen( ));

            if( _master )
            {

                eq::net::Barrier barrier( node, 2 );
                node->registerObject( &barrier );
                TEST( barrier.isAttached( ));

                _barrier = &barrier;
                barrier.enter();

                _barrier.waitEQ( 0 ); // wait for slave to unmap session
                node->deregisterObject( &barrier );
            }
            else
            {
                _barrier.waitNE( 0 );

                eq::net::NodePtr server = new eq::net::Node;
                eq::net::ConnectionDescriptionPtr serverDesc = 
                    new eq::net::ConnectionDescription;
                serverDesc->port = 4242;
                server->addConnectionDescription( serverDesc );
                TEST( node->connect( server ));

                std::cerr << "Slave enter" << std::endl;
                _barrier->enter();
                std::cerr << "Slave left" << std::endl;

                _barrier = 0;
            }

            node->close();
        }
            
private:
    bool _master;
};

int main( int argc, char **argv )
{
    TEST( eq::net::init( argc, argv ));

    NodeThread server( true );
    NodeThread node( false );

    server.start();
    node.start();
    server.join();
    node.join();

    eq::net::exit();
    return EXIT_SUCCESS;
}

