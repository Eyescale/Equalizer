
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_RNG_H
#define EQBASE_RNG_H

#include <eq/base/debug.h> // for EQASSERT
#include <fcntl.h>

namespace eqBase
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
#endif  // EQBASE_RNG_H
