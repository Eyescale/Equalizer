
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_UUID_H
#define COBASE_UUID_H

#include <co/base/rng.h>     // used in inline method
#include <co/base/uint128_t.h> // base class

namespace co
{
namespace base
{

    /**
     * Provides a universally unique identifier.
     *
     * Not to be subclassed.
     */
    class UUID : public uint128_t
    {
    public:
        /** 
         * Construct a new universally unique identifier.
         *
         * If generate is set to true, a new UUID is allocated. Otherwise the
         * UUID is cleared, i.e., it is equal to UUID::ZERO.
         * @version 1.0
         */
        explicit UUID( const bool generate = false ) : uint128_t()
            { 
                while( generate && high() == 0 )
                {         
                    RNG rng;
                    high() = rng.get< uint64_t >();
                    low() = rng.get< uint64_t >();
                }
            }

        /**
         * Construct a new universally unique identifier.
         * @version 1.0
         */
        UUID( const uint64_t high_, const uint64_t low_ ) 
            : uint128_t( high_, low_ ) {}

        /**
         * Construct a new universally unique identifier from an unsigned
         * 128 bit integer value.
         * @version 1.0
         */
        UUID( const uint128_t& from ) : uint128_t( from ) {}

        /** Assign another UUID from a string representation. @version 1.0 */
        UUID& operator = ( const std::string& from )
        {
            char* next = 0;
#ifdef _MSC_VER
            high() = ::_strtoui64( from.c_str(), &next, 16 );
#else
            high() = ::strtoull( from.c_str(), &next, 16 );
#endif
            EQASSERT( next != from.c_str( ));
            EQASSERTINFO( *next == ':', from << ", " << next );

            ++next;
#ifdef _MSC_VER
            low() = ::_strtoui64( next, 0, 16 );
#else
            low() = ::strtoull( next, 0, 16 );
#endif
            return *this;
        }

        /** @return true if the UUID was generated. */
        bool isGenerated() const { return high() != 0; }

        /** The NULL UUID. @version 1.0 */
        static COBASE_API const UUID ZERO;
#ifdef _MSC_VER
        friend size_t stde::hash_compare< co::base::UUID >::operator() 
            ( const co::base::UUID& key ) const;
#else
        friend struct stde::hash< co::base::UUID >;
#endif
    };

    /** A hash for UUID keys. @version 1.0 */
    template<class T> class UUIDHash : public stde::hash_map< UUID, T > {};


}
}

#endif // COBASE_UUID_H
