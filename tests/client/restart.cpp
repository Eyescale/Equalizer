
/* Copyright (c) 2016, Stefan.Eilemann@epfl.ch
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

// https://github.com/Eyescale/Equalizer/issues/539

#include <lunchbox/test.h>
#include <eq/eq.h>

#ifdef _WIN32
#  define setenv( name, value, overwrite ) \
    _putenv_s( name, value )
#endif

#ifdef EQUALIZER_USE_HWSD
#define LOOPS 2

int main( const int argc, char** argv )
{
#ifdef Darwin
    ::setenv( "EQ_WINDOW_IATTR_HINT_DRAWABLE", "-8" /*PBuf*/, 1 /*overwrite*/ );
#else
    ::setenv( "EQ_WINDOW_IATTR_HINT_DRAWABLE", "-12" /*FBO*/, 1 /*overwrite*/ );
#endif
    eq::NodeFactory nodeFactory;
    TEST( eq::init( argc, argv, &nodeFactory ));

    for( size_t i = 0; i < LOOPS; ++i )
    {
        eq::ClientPtr client = new eq::Client;
        TEST( client->initLocal( argc, argv ));

        for( size_t j = 0; j < LOOPS; ++j )
        {
            eq::ServerPtr server = new eq::Server;
            TEST( client->connectServer( server ));

            for( size_t k = 0; k < LOOPS; ++k )
            {
                eq::fabric::ConfigParams configParams;
                eq::Config* config = server->chooseConfig( configParams );
                if( !config ) // Autoconfig failed, likely there are no GPUs
                    continue;

                for( size_t l = 0; l < LOOPS; ++l )
                {
                    TESTINFO( config->init( co::uint128_t( )),
                              "at client " << i << " server " << j <<
                              " config " << k << " init " << l );
                    config->exit();
                }
                server->releaseConfig( config );
            }
            client->disconnectServer( server );
        }
        client->exitLocal();
    }
    eq::exit();
    return EXIT_SUCCESS;
}

#else

int main( const int, char** )
{
    return EXIT_SUCCESS;
}

#endif
