
/* Copyright (c) 2014, Stefan.Eilemann@epfl.ch
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

// https://github.com/Eyescale/Equalizer/issues/304

#include <lunchbox/test.h>
#include <eq/eq.h>

int main( const int argc, char** argv )
{
    eq::Global::setConfigFile( "configs/issue304.eqc" );

    eq::NodeFactory nodeFactory;
    TEST( eq::init( argc, argv, &nodeFactory ));

    eq::ClientPtr client = new eq::Client;
    TEST( client->initLocal( argc, argv ));

    eq::ServerPtr server = new eq::Server;
    TEST( client->connectServer( server ));

    eq::fabric::ConfigParams configParams;
    eq::Config* config = server->chooseConfig( configParams );
    TEST( config );

    TEST( !config->init( eq::uint128_t( )));
    TEST( !config->isRunning( ));
    TEST( config->getErrors().size() == 3 );
    TEST( config->getErrors().empty( ));

    server->releaseConfig( config );
    TEST( client->disconnectServer( server ));
    client->exitLocal();

    TESTINFO( client->getRefCount() == 1, client->getRefCount( ));
    TESTINFO( server->getRefCount() == 1, server->getRefCount( ));

    TEST( eq::exit( ));
    return EXIT_SUCCESS;
}
