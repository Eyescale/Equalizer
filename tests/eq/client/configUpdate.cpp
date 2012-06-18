
/* Copyright (c) 2012, Stefan Eilemann <eile@eyescale.ch> 
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

// https://github.com/Eyescale/Equalizer/issues/130

#include <test.h>
#include <eq/eq.h>

#define RUNTIME 1000 // ms

int main( const int argc, char** argv )
{
    eq::NodeFactory nodeFactory;
    TEST( eq::init( argc, argv, &nodeFactory ));

    eq::Config* config = eq::getConfig( argc, argv );
    TEST( config );

    size_t nLoops = 0;
    const lunchbox::Clock clock;
    while( clock.getTime64() < RUNTIME )
    {
        TEST( config->update( ));
        ++nLoops;
    }
    const float time = clock.getTimef();

    std::cout << nLoops << " Config::update in " << time << " ms (" << 
        time/float(nLoops) << " ms/update)" << std::endl;

    eq::releaseConfig( config );
    eq::exit();
    return EXIT_SUCCESS;
}
