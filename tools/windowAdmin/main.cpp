
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
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

#include <eq/admin/admin.h>

#include <admin/addWindow.h>

void _runMainLoop( eq::admin::ServerPtr server );

int main( const int argc, char** argv )
{
    // 1. Equalizer admin initialization
    if( !eq::admin::init( argc, argv ))
    {
        LBERROR << "Initialization of Equalizer administrative library failed"
                << std::endl;
        return EXIT_FAILURE;
    }

    // 2. initialization of local client node
    eq::admin::ClientPtr client = new eq::admin::Client;
    if( !client->initLocal( argc, argv ))
    {
        LBERROR << "Can't init client" << std::endl;
        eq::admin::exit();
        return EXIT_FAILURE;
    }

    // 3. connect to server
    eq::admin::ServerPtr server = new eq::admin::Server;
    if( !client->connectServer( server ))
    {
        LBERROR << "Can't open server" << std::endl;
        eq::admin::exit();
        return EXIT_FAILURE;
    }

    _runMainLoop( server );

    // 4. cleanup and exit
    if( !client->disconnectServer( server ))
        LBERROR << "Client::disconnectServer failed" << std::endl;

    client->exitLocal();

    LBASSERTINFO( client->getRefCount() == 1, client->getRefCount( ));
    LBASSERTINFO( server->getRefCount() == 1, server->getRefCount( ));
    client = 0;
    eq::admin::exit();
    return EXIT_SUCCESS;
}

void _runMainLoop( eq::admin::ServerPtr server )
{
    eqAdmin::addWindow( server, false /* active stereo */ );
}
