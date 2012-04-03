
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

#include <eq/server/global.h>
#include <eq/server/loader.h>
#include <eq/server/server.h>

#include <lunchbox/file.h>
#include <lunchbox/init.h>

#include <locale.h>

// Tests (re)loading of all examples/configs/*.eqc files
int main( int argc, char **argv )
{
    // Test for: https://github.com/Eyescale/Equalizer/issues/56
    setlocale( LC_ALL, "de_DE.UTF-8" ); 

    TEST( lunchbox::init( argc, argv ));

    eq::server::Loader loader;
    lunchbox::Strings configs = lunchbox::searchDirectory( "configs", "*.eqc" );
    TESTINFO( configs.size() > 20, configs.size( ));

    for( lunchbox::StringsCIter i = configs.begin(); i != configs.end(); ++i )
    {
        const std::string& filename = "configs/" + *i;
        eq::server::Global* global = eq::server::Global::instance();
        const eq::server::Config::FAttribute attr = 
            eq::server::Config::FATTR_VERSION;

        // load
        global->setConfigFAttribute( attr, 0.f );
        eq::server::ServerPtr server = loader.loadFile( filename );
        TESTINFO( server.isValid(), "Load of " << filename << " failed" );
        TESTINFO( global->getConfigFAttribute( attr ) == 1.1f ||
                  global->getConfigFAttribute( attr ) == 1.2f,
                  global->getConfigFAttribute( attr ) << " in " << filename );

        // convert
        eq::server::Loader::addOutputCompounds( server );
        eq::server::Loader::addDestinationViews( server );
        eq::server::Loader::addDefaultObserver( server );
        eq::server::Loader::convertTo11( server );
        eq::server::Loader::convertTo12( server );

        // output
        std::ofstream logFile( "testOutput.eqc" );
        TEST( logFile.is_open( ));

        std::ostream& oldOut = lunchbox::Log::getOutput();
        lunchbox::Log::setOutput( logFile );
        OUTPUT << eq::server::Global::instance() << *server
               << lunchbox::forceFlush;
        lunchbox::Log::setOutput( oldOut );
        OUTPUT << lunchbox::enableHeader << std::endl;
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

    TEST( lunchbox::exit( ));
    return EXIT_SUCCESS;
}
