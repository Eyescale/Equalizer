
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "libs/server/loader.h"
#include "libs/server/server.h"

#include <co/base/init.h>

// Tests restarting the loader after a parse error

int main( int argc, char **argv )
{
    eq::server::Loader loader;

    TEST( co::base::init( argc, argv ));
    TEST( !loader.loadFile( argv[0] ));
    TEST( loader.loadFile( "../../examples/configs/config.eqc" ).isValid( ))

    TEST( co::base::exit( ));
    return EXIT_SUCCESS;
}
