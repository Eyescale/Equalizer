
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "server.h"

#include "global.h"
#include "loader.h"

#include <eq/net/global.h>
#include <eq/net/init.h>

#include <iostream>

using namespace eq::server;
using namespace eq::base;
using namespace std;

#define CONFIG "server{ config{ appNode{ pipe {                            \
    window { viewport [ .25 .25 .5 .5 ] channel { name \"channel\" }}}}    \
    compound { channel \"channel\" wall { bottom_left  [ -.8 -.5 -1 ]      \
                                          bottom_right [  .8 -.5 -1 ]      \
                                          top_left     [ -.8  .5 -1 ] }}}}"

int main( const int argc, char** argv )
{
    eq::net::init( argc, argv );
    eq::net::Global::setDefaultPort( EQ_DEFAULT_PORT );

    Loader loader;
    RefPtr<Server> server;

    if( argc == 1 )
    {
        server = loader.parseServer( CONFIG );
    }
    else
    {
        server = loader.loadFile( argv[1] );
    }

    if( !server.isValid( ))
    {
        EQERROR << "Server load failed" << endl;
        return EXIT_FAILURE;
    }

    Loader::addOutputCompounds( server );
    Loader::addDestinationViews( server );
    Loader::addDefaultObserver( server );

    if( !server->initLocal( argc, argv ))
    {
        EQERROR << "Can't create listener for server, please consult log" 
                << endl;
        return EXIT_FAILURE;
    }

    if( !server->run( ))
    {
        EQERROR << "Server did not run correctly, please consult log" << endl;
        return EXIT_FAILURE;
    }

    server->exitLocal();

    EQINFO << "Server ref count: " << server->getRefCount() << endl;
    return EXIT_SUCCESS;
}

