
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifndef EQBASE_RNG_H
#define EQBASE_RNG_H

#include <eq/base/debug.h> // for EQASSERT
#include <fcntl.h>

namespace eq
{
namespace base
{
    /** A random number generator */
    class EQ_EXPORT RNG
    {
    public:
        RNG()
        {
#ifdef Linux
            _fd = ::open( "/dev/urandom", O_RDONLY );
            EQASSERT( _fd != -1 );
#endif
            reseed();
        }

        ~RNG()
        {
#ifdef Linux
            if( _fd > 0 )
                close( _fd );
#endif
        }

        void reseed()
        {
#ifdef Linux
            // NOP
#elif defined (WIN32)
            LARGE_INTEGER seed;
            QueryPerformanceCounter( &seed );
            srand( seed.LowPart );
#else // Darwin
            srandomdev();
#endif
        }

        template< typename T >
        T get()
        {
            T              value;
#ifdef Linux
            int read = ::read( _fd, &value, sizeof( T ));
            EQASSERT( read == sizeof( T ));
            if( read != sizeof( T ))
            {
                EQERROR << "random number generator not working" << std::endl;
                return 0;
            }

#elif defined (WIN32)

            EQASSERTINFO( RAND_MAX >= 32767, RAND_MAX );

            unsigned char* bytes = reinterpret_cast< unsigned char* >( &value );
            for( size_t i=0; i<sizeof( T ); ++i )
                bytes[i] = ( rand() & 255 );
#else // Darwin
            unsigned char* bytes = 
                reinterpret_cast< unsigned char* >( &value );
            for( size_t i=0; i<sizeof( T ); ++i )
                bytes[i] = ( random() & 255 );
#endif
            return value;
        }

    private:
#ifdef Linux
        int _fd;
#endif
    };
}
}
#endif  // EQBASE_RNG_H
