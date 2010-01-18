
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

#ifndef EQBASE_UUID_H
#define EQBASE_UUID_H

#include <eq/base/hash.h>
#include <eq/base/log.h>
#include <eq/base/rng.h>

namespace eq
{
namespace base
{
    /**
     * Provides a universally unique identifier.
     *
     * Not to be subclassed.
     */
    class EQ_EXPORT UUID
    {
    public:
        /** 
         * Construct a new universally unique identifier.
         *
         * If generate is set to true, a new UUID is allocated. Otherwise the
         * UUID is cleared, i.e., it is equal to UUID::ZERO.
         * @version 1.0
         */
        UUID( const bool generate = false ) : _high( 0 ), _low( 0 )
            { 
                if( generate )
                {         
                    RNG rng;
                    _high = rng.get< uint64_t >();
                    _low = rng.get< uint64_t >();
                }
            }

        /** Create a copy of a universally unique identifier. @version 1.0 */
        UUID( const UUID& from ) : _high( from._high ), _low( from._low ) {}

        /** Assign another universally unique identifier. @version 1.0 */
        UUID& operator = ( const UUID& from )
            {
                _high = from._high;
                _low = from._low;
                return *this;
            }

        /** Assign another UUID from a string representation. @version 1.0 */
        UUID& operator = ( const std::string& from )
            {
                char* next = 0;
#ifdef WIN32_VC
                _high = ::_strtoui64( from.c_str(), &next, 16 );
#else
                _high = ::strtoull( from.c_str(), &next, 16 );
#endif
                EQASSERT( next != from.c_str( ));
                EQASSERTINFO( *next == ':', from << ", " << next );

                ++next;
#ifdef WIN32_VC
                _low = ::_strtoui64( next, 0, 16 );
#else
                _low = ::strtoull( next, 0, 16 );
#endif
                return *this;
            }

        /** @return true if the UUIDs are equal, false if not. @version 1.0 */
        bool operator == ( const UUID& rhs ) const
            { return _high == rhs._high && _low == rhs._low; }

        /**
         * @return true if the UUIDs are different, false otherwise.
         * @version 1.0
         */
        bool operator != ( const UUID& rhs ) const
            { return _high != rhs._high || _low != rhs._low; }

        /**
         * @return true if this UUID is smaller than the RHS UUID.
         * @version 1.0
         */
        bool operator < ( const UUID& rhs ) const
            { 
                if( _high < rhs._high )
                    return true;
                if( _high > rhs._high )
                    return false;
                return _low < rhs._low; 
            }

        /**
         * @return true if this UUID is bigger than the rhs UUID.
         * @version 1.0
         */
        bool operator > ( const UUID& rhs ) const
            { 
                if( _high > rhs._high )
                    return true;
                if( _high < rhs._high )
                    return false;
                return _low > rhs._low; 
            }

        /** The NULL UUID. @version 1.0 */
        static const UUID ZERO;

    private:
        uint64_t _high;
        uint64_t _low;

        friend std::ostream& operator << ( std::ostream& os, const UUID& id );

#ifdef WIN32_VC
        friend size_t stde::hash_compare< eq::base::UUID >::operator() 
            ( const eq::base::UUID& key ) const;
        friend size_t stde::hash_value( const eq::base::UUID& key );
#else
        friend struct stde::hash< eq::base::UUID >;
#endif
    };

    /** A hash for UUID keys. @version 1.0 */
    template<class T> class UUIDHash : public stde::hash_map< UUID, T > {};

    /** UUID& ostream operator. */
    inline std::ostream& operator << ( std::ostream& os, const UUID& id )
    {
        os << std::hex << id._high << ':' << id._low << std::dec;
        return os;
    }
}
}

#ifdef WIN32_VC
template<> inline size_t stde::hash_compare< eq::base::UUID >::operator() 
    ( const eq::base::UUID& key ) const
{
    return key._low;
}

template<> inline size_t stde::hash_value( const eq::base::UUID& key )
{
    return key._low;
}

#elif defined (WIN32)

namespace __gnu_cxx
{
    template<> struct hash< eq::base::UUID >
    {
        size_t operator()( const eq::base::UUID& key ) const
        {
            return key._low;
        }
    };
}

#else // POSIX

#  ifdef __GNUC__              // GCC 3.1 and later
#    ifdef EQ_GCC_4_2_OR_LATER
namespace std { namespace tr1
#    else
namespace __gnu_cxx
#    endif
#  else //  other compilers
namespace std
#  endif
{
    template<> struct hash< eq::base::UUID >
    {
        size_t operator()( const eq::base::UUID& key ) const
        {
            return key._low;
        }
    };
#ifdef EQ_GCC_4_2_OR_LATER
}
#endif
}

#endif // WIN32_VC
#endif // EQBASE_NODE_H
