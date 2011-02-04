
/* Copyright (c) 2009-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#if defined(_MSC_VER)
static const std::string CONFIG_DIR = "../../../examples/configs/";
#else
static const std::string CONFIG_DIR = "../../examples/configs/";
#endif
// Tests (re)loading of all examples/configs/*.eqc files

int main( int argc, char **argv )
{
    TEST( co::base::init( argc, argv ));

    eq::server::Loader loader;
    co::base::Strings candidates = co::base::searchDirectory( CONFIG_DIR,
                                                              "*.eqc" );
    for( co::base::Strings::const_iterator i = candidates.begin();
        i != candidates.end(); ++i )
    {
        const std::string& filename = CONFIG_DIR + *i;
        eq::server::Global* global = eq::server::Global::instance();
        const eq::server::Config::FAttribute attr = 
            eq::server::Config::FATTR_VERSION;

        // load
        global->setConfigFAttribute( attr, 0.f );
        eq::server::ServerPtr server = loader.loadFile( filename );
        TESTINFO( server.isValid(), "Load of " << filename << " failed" );
        TESTINFO( global->getConfigFAttribute( attr ) == 1.f ||
                  global->getConfigFAttribute( attr ) == 1.1f,
                  global->getConfigFAttribute( attr ) << " file " << filename);

        // convert
        eq::server::Loader::addOutputCompounds( server );
        eq::server::Loader::addDestinationViews( server );
        eq::server::Loader::addDefaultObserver( server );
        eq::server::Loader::convertTo11( server );

        // output
        std::ofstream logFile( "testOutput.eqc" );
        TEST( logFile.is_open( ));

        std::ostream& oldOut = co::base::Log::getOutput();
        co::base::Log::setOutput( logFile );
        OUTPUT << eq::server::Global::instance() << *server
               << co::base::forceFlush;
        co::base::Log::setOutput( oldOut );
        OUTPUT << co::base::enableHeader << std::endl;
        logFile.close();

        // cleanup
        eq::server::Global::clear();
        server->deleteConfigs(); // break server <-> config ref circle
        TESTINFO( server->getRefCount() == 1,
                  server->getRefCount() << ": " << server );

        // reload output
        server = loader.loadFile( "testOutput.eqc" );
        TESTINFO( server.isValid(), "Output/reload of " << filename <<
                  " failed, see testOutput.eqc" );

        eq::server::Global::clear();
        server->deleteConfigs(); // break server <-> config ref circle
        TESTINFO( server->getRefCount() == 1,
                  server->getRefCount() << ": " << server );
    }

    TEST( co::base::exit( ));
    return EXIT_SUCCESS;
}
