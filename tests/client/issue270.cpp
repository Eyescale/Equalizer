
/* Copyright (c) 2013, Stefan.Eilemann@epfl.ch
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

// https://github.com/Eyescale/Equalizer/issues/270

#include <lunchbox/test.h>
#include <eq/eq.h>

int main( const int argc, char** argv )
{
    eq::Global::setConfigFile( "configs/config.eqc" );

    eq::NodeFactory nodeFactory;
    TEST( eq::init( 0, 0, &nodeFactory ));

    eq::ClientPtr client = new eq::Client;
    TEST( client->initLocal( argc, argv ));

    eq::ServerPtr server = new eq::Server;
    TEST( client->connectServer( server ));

    eq::fabric::ConfigParams configParams;
    configParams.setFlags( eq::fabric::ConfigParams::FLAG_MULTIPROCESS_DB );

    eq::Config* config = server->chooseConfig( configParams );
    TEST( config );

    server->releaseConfig( config );
    client->disconnectServer( server );
    client->exitLocal();

    TESTINFO( client->getRefCount() == 1, client->getRefCount( ));
    TESTINFO( server->getRefCount() == 1, server->getRefCount( ));

    eq::exit();
    return EXIT_SUCCESS;
}
