
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_RNG_H
#define COBASE_RNG_H

#include <co/base/debug.h> // for EQASSERT
#include <co/base/init.h>  // friend function
#include <co/base/nonCopyable.h>
#include <co/base/types.h>

#pragma warning (push)
#pragma warning (disable: 4985) // inconsistent decl of ceil

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <wtypes.h>
#  include <wincrypt.h>
#  pragma comment(lib, "advapi32.lib")
#endif

#include <fcntl.h>
#include <limits>
#include <stdio.h>
#pragma warning (pop)

namespace co
{
namespace base
{
    /**
     * A random number generator.
     *
     * Generates a set of random, or if not supported by the operating system,
     * pseudo-random numbers. Each instance creates its own series of numbers.
     */
    class RNG : public NonCopyable
    {
    public:
        /** Construct a new random number generator. @version 1.0 */
        RNG() { _init(); }

        /** Destruct the random number generator. @version 1.0 */
        ~RNG() {}

        /** Re-initialize the seed value for pseudo RNG's. @version 1.0 */
        void reseed()
        {
#ifdef Darwin
            srandomdev();
#endif
        }

        /**
          * Generate a random number.
          * 
          * The returned number is between min..max for integer types, and 
          * between 0..1 for floating-point types.
          * @return a random number.
          * @version 1.0 
          */
        template< typename T > T get()
        {
            T value;
#ifdef Linux
            EQASSERTINFO( _fd >= 0, "init() not called?" );
            int read = ::read( _fd, &value, sizeof( T ));
            EQASSERTINFO( read == sizeof(T),
                          read << " != " << sizeof( T ) << ": " << sysError );
            if( read != sizeof( T ))
            {
                EQERROR << "random number generator not working" << std::endl;
                return 0;
            }

#elif defined (_WIN32)
            EQASSERTINFO( _provider, "init() not called?" );
            if( !CryptGenRandom( _provider, sizeof( T ), (BYTE*)&value ))
            {
                EQASSERTINFO( false, "random number generator not working: " <<
                                     sysError );
            }
#else // Darwin
            uint8_t* bytes = reinterpret_cast< uint8_t* >( &value );
            for( size_t i=0; i<sizeof( T ); ++i )
                bytes[i] = ( random() & 0xff );
#endif
            return value;
        }

    private:
#ifdef Linux
        static int _fd;
#elif defined (_WIN32)
        static COBASE_API HCRYPTPROV _provider;
#endif
        static void _init();
        static void _exit();
        friend COBASE_API bool init( const int argc, char** argv );
    };

    template<> inline float RNG::get()
    {
        const float max_limits =
            static_cast< float >( std::numeric_limits< uint32_t >::max( ));
        return ( get< uint32_t >() / max_limits);
    }
    
    template<> inline double RNG::get()
    {
        const double max_limits =
            static_cast< double >( std::numeric_limits< uint64_t >::max( ));
        return ( get< uint64_t >() / max_limits);
    }
    
    template<> inline bool RNG::get()
    {
        return ( get< uint32_t >() & 1 );
    }
}
}
#endif  // COBASE_RNG_H
