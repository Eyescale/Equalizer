
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQTEST_TEST_H
#define EQTEST_TEST_H

#include <eq/base/log.h>
#include <eq/base/sleep.h>
#include <eq/base/thread.h>
#include <fstream>

#define OUTPUT eq::base::Log::instance( SUBDIR, __FILE__, __LINE__ )

#define TEST( x )                                                       \
    {                                                                   \
        EQVERB << "Test " << #x << std::endl;                           \
        if( !(x) )                                                      \
        {                                                               \
            OUTPUT << #x << " failed (l." << __LINE__ << ')' << std::endl; \
            ::exit( EXIT_FAILURE );                                     \
        }                                                               \
    }

#define TESTINFO( x, info )                                           \
    {                                                                 \
        EQVERB << "Test " << #x << ": " << info << std::endl;         \
        if( !(x) )                                                    \
        {                                                             \
            OUTPUT << #x << " failed (l." << __LINE__ << "): " << info  \
                   << std::endl;                                        \
            ::exit( EXIT_FAILURE );                                     \
        }                                                               \
    }

int testMain( int argc, char **argv );

namespace
{
class Watchdog : public eq::base::Thread
{
public:
    Watchdog( const std::string& name ) : _name( name ) {}

    virtual void run()
        {
#ifdef EQ_TEST_RUNTIME
            eq::base::sleep( EQ_TEST_RUNTIME * 1000 );
            TESTINFO( false, 
                      "Watchdog triggered - " << _name <<
                      " did not terminate within " << EQ_TEST_RUNTIME << "s" );
#else
            eq::base::sleep( 60000 );
            TESTINFO( false, 
                      "Watchdog triggered - " << _name <<
                      " did not terminate within 1 minute" );
#endif
        }

private:
    const std::string _name;
};

}

int main( int argc, char **argv )
{
#ifndef EQ_TEST_NO_WATCHDOG
    Watchdog watchdog( argv[0] );
    watchdog.start();
#endif

    const int result = testMain( argc, argv );
    if( result != EXIT_SUCCESS )
        return result;

    const std::string filename( argv[ 0 ] + std::string( ".testOK" ));

    std::ofstream file( filename.c_str(), std::ios::out | std::ios::binary );
    if( file.is_open( ))
    {
        file.write( filename.c_str(), filename.length( ));
        file.close();
    }

    return EXIT_SUCCESS;
}

#  define main testMain

#endif // EQTEST_TEST_H

