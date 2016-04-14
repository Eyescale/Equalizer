
/* Copyright (c) 2013, Julio Delgado Mangas <julio.delgadomangas@epfl.ch>
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
#include <lunchbox/test.h>
#include <eq/eq.h>
#include <eq/fabric/channel.h>

#include <lunchbox/memoryMap.h>
#include <boost/filesystem.hpp>
#include <cstdio>
#include <fstream>
#include <string>

#ifdef _WIN32
#  define setenv( name, value, overwrite ) \
    _putenv_s( name, value )
#endif

#ifdef EQUALIZER_USE_HWSD
static const unsigned int WIDTH = 200;
static const unsigned int HEIGHT = 100;
static const unsigned int BYTES_PER_PIXEL = 4;
static const unsigned int HEADER_SIZE = 512;
static const size_t EXPECTED_SIZE = ( WIDTH * HEIGHT * BYTES_PER_PIXEL )
                                      + HEADER_SIZE;

static const std::string PREFIX = "prefix_";
static const std::string GENERATED_FILENAME = PREFIX + "1.rgb";

namespace
{

class TestWindow: public eq::Window
{
public:
    TestWindow( eq::Pipe* parent )
        : eq::Window( parent )
    {
    }

    virtual bool configInit( const co::uint128_t& initID )
    {
        setPixelViewport( eq::PixelViewport( 0, 0, WIDTH, HEIGHT ));
        return eq::Window::configInit( initID );
    }
};

class TestNodeFactory: public eq::NodeFactory
{
public:
    virtual eq::Window* createWindow( eq::Pipe* parent )
    {
        return new TestWindow( parent );
    }
};
}

/*
 * Tests the ability to dump views to a file.
 */
int main( const int argc, char** argv )
{
    // 1.- Prepare test
    ::setenv( "EQ_CHANNEL_SATTR_DUMP_IMAGE", PREFIX.c_str(), 1 /*overwrite*/ );
#ifndef Darwin
    ::setenv( "EQ_WINDOW_IATTR_HINT_DRAWABLE", "-12" /*FBO*/, 1 /*overwrite*/ );
#endif

    // 2.- Start application
    TestNodeFactory nodeFactory;
    TEST( eq::init( argc, argv, &nodeFactory ));

    eq::ClientPtr client = new eq::Client;
    TEST( client->initLocal( argc, argv ));

    eq::ServerPtr server = new eq::Server;
    TEST( client->connectServer( server ));

    eq::fabric::ConfigParams configParams;
    eq::Config* config = server->chooseConfig( configParams );

    if( !config ) // Most probably no GPUs present, tests in meaningless
    {
        client->disconnectServer( server );
        client->exitLocal();
        eq::exit();
        return EXIT_SUCCESS;
    }
    TEST( config->init( co::uint128_t( )));

    // 3.- Force frame generation
    config->startFrame( co::uint128_t( ));
    config->finishFrame();

    // 4.- Make sure EQ is properly shut down
    config->exit();
    server->releaseConfig( config );
    client->disconnectServer( server );
    client->exitLocal();
    eq::exit();

    // 5.- Verify results
    TESTINFO( boost::filesystem::exists( GENERATED_FILENAME ),
              GENERATED_FILENAME );
    TESTINFO( EXPECTED_SIZE == boost::filesystem::file_size( GENERATED_FILENAME ),
              EXPECTED_SIZE << " != " <<
              boost::filesystem::file_size( GENERATED_FILENAME ));

    // 6.- Remove generated file
    ::remove( GENERATED_FILENAME.c_str( ));

    return EXIT_SUCCESS;
}

#else

int main( const int, char** )
{
    return EXIT_SUCCESS;
}

#endif
