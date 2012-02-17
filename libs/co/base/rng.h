
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

namespace co
{
namespace base
{
namespace detail { class RNG; }

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
        COBASE_API RNG();

        /** Destruct the random number generator. @version 1.0 */
        COBASE_API ~RNG();

        /** Re-initialize the seed value for pseudo RNG's. @version 1.0 */
        COBASE_API void reseed();

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
            if( !_get( &value, sizeof( T )))
                return 0;
            return value;
        }

    private:
        detail::RNG* const _impl;

        static COBASE_API bool _init();
        static void _exit();
        friend COBASE_API bool init( const int argc, char** argv );
        COBASE_API bool _get( void* data, size_t size );
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
