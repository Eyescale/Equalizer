
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

#include "libs/server/global.h"
#include "libs/server/loader.h"
#include "libs/server/server.h"

#include <co/base/file.h>
#include <co/base/init.h>

// Tests loading of all examples/configs/*.eqc files

int main( int argc, char **argv )
{
    TEST( co::base::init( argc, argv ));

    eq::server::Loader loader;
    co::base::Strings candidates = 
        co::base::searchDirectory( "../../examples/configs", "*.eqc" );
    for( co::base::Strings::const_iterator i = candidates.begin();
        i != candidates.end(); ++i )
    {
        const std::string& filename = "../../examples/configs/" + *i;
        eq::server::Global* global = eq::server::Global::instance();
        const eq::server::Config::FAttribute attr = 
            eq::server::Config::FATTR_VERSION;

        global->setConfigFAttribute( attr, 0.f );
        eq::server::ServerPtr server = loader.loadFile( filename );

        TESTINFO( server.isValid(), "Load of " << filename << " failed" );
        TESTINFO( global->getConfigFAttribute( attr ) == 1.f ||
                  global->getConfigFAttribute( attr ) == 1.1f,
                  global->getConfigFAttribute( attr ) << " file " << filename);
    }

    TEST( co::base::exit( ));
    return EXIT_SUCCESS;
}
