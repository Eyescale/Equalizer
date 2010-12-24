
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef COBASE_UINT128_H
#define COBASE_UINT128_H

#include <co/base/stdExt.h>
#include <sstream>

#ifdef _MSC_VER
// Don't include <co/base/types.h> to be minimally intrusive for apps
// using uint128_t
#  include <basetsd.h>
typedef UINT64     uint64_t;
#else
#  include <stdint.h>
#endif

namespace co
{
namespace base
{
    /** A base type for 128 bit unsigned integer values. */
    class uint128_t
    {
    public:
        /**
         * Construct a new 128 bit integer with a default value.
         * @version 1.0
         */
        uint128_t( const uint64_t low_ = 0 ) 
            : _high( 0 ), _low( low_ ) {}

        /**
         * Construct a new 128 bit integer with default values.
         * @version 1.0
         **/
        uint128_t( const uint64_t high_, const uint64_t low_ ) 
            : _high( high_ ), _low( low_ ) {}

        /** Assign another 128 bit value. @version 1.0 */
        uint128_t& operator = ( const uint128_t& rhs )
            {
                _high = rhs._high;
                _low = rhs._low;
                return *this;
            }

        /**
         * @return true if the values are equal, false if not.
         * @version 1.0
         **/
        bool operator == ( const uint128_t& rhs ) const
            { return _high == rhs._high && _low == rhs._low; }

        /**
         * @return true if the values are different, false otherwise.
         * @version 1.0
         **/
        bool operator != ( const uint128_t& rhs ) const
            { return _high != rhs._high || _low != rhs._low; }

        /**
         * @return true if this value is smaller than the RHS value.
         * @version 1.0
         **/
        bool operator < ( const uint128_t& rhs ) const
            { 
                if( _high < rhs._high )
                    return true;
                if( _high > rhs._high )
                    return false;
                return _low < rhs._low; 
            }

        /**
         * @return true if this value is bigger than the rhs value.
         * @version 1.0
         */
        bool operator > ( const uint128_t& rhs ) const
            { 
                if( _high > rhs._high )
                    return true;
                if( _high < rhs._high )
                    return false;
                return _low > rhs._low; 
            }

        /**
         * @return true if this value is smaller or equal than the
         *         RHS value.
         * @version 1.0
         */
        bool operator <= ( const uint128_t& rhs ) const
            { 
                if( _high < rhs._high )
                    return true;
                if( _high > rhs._high )
                    return false;
                return _low <= rhs._low; 
            }

        /**
         * @return true if this value is smaller or equal than the
         *         RHS value.
         * @version 1.0
         */
        bool operator >= ( const uint128_t& rhs ) const
            { 
                if( _high > rhs._high )
                    return true;
                if( _high < rhs._high )
                    return false;
                return _low >= rhs._low; 
            }

        /** Increment the value. @version 1.0 */
        uint128_t& operator ++()
            { 
                ++_low;
                if( !_low )
                    ++_high;
                
                return *this;
            }
        
        /** Decrement the value. @version 1.0 */
        uint128_t& operator --()
            { 
                if( !_low )
                    --_high;
                --_low;
                return *this;
            }

        /** @return the reference to the lower 64 bits of this 128 bit value. */
        const uint64_t& low() const { return _low; }
        /** @return the reference to the high 64 bits of this 128 bit value. */
        const uint64_t& high() const { return _high; }

        /** @return the reference to the lower 64 bits of this 128 bit value. */
        uint64_t& low() { return _low; }
        /** @return the reference to the high 64 bits of this 128 bit value. */
        uint64_t& high() { return _high; }

        /** @return a short, but not necessarily unique, string of the value. */
        std::string getShortString() const
            {
                std::stringstream stream;
                stream << std::hex << _high << _low;
                const std::string str = stream.str();
                return str.substr( 0, 3 ) + ".." +
                    str.substr( str.length() - 3, std::string::npos );
            }
    private:
        uint64_t _high;
        uint64_t _low;
    };

    /** ostream operator for 128 bit unsigned integers. @version 1.0 */
    inline std::ostream& operator << ( std::ostream& os, const uint128_t& id )
    {
        os << std::hex << id.high() << ':' << id.low() << std::dec;
        return os;
    }

    /** Add a 64 bit value to a 128 bit value. @version 1.0 */
    inline uint128_t operator+ ( const uint128_t& a, const uint64_t& b ) 
    {
        uint128_t result = a;
        result.low() += b;
        if( result.low() < a.low( ))
            ++result.high();
        return result;
    };

    /** Substract a 64 bit value from a 128 bit value. @version 1.0 */
    inline uint128_t operator- ( const uint128_t& a, const uint64_t& b ) 
    {
        uint128_t result = a;
        result.low() -= b;
        if( result.low() > a.low( ))
            --result.high();
        return result;
    };

}
}

#ifdef EQ_STDEXT_VC8

template<> inline size_t stde::hash_compare< co::base::uint128_t >::operator() 
        ( const co::base::uint128_t& key ) const
{
    return key.high() ^ key.low();
}

template<> inline size_t stde::hash_value( const co::base::uint128_t& key )
{
    return key.high() ^ key.low();
}

#else // EQ_STDEXT_VC8

EQ_STDEXT_NAMESPACE_OPEN
template<> struct hash< co::base::uint128_t >
{
    size_t operator()( const co::base::uint128_t& key ) const
        {
            return key.high() ^ key.low();
        }
};
EQ_STDEXT_NAMESPACE_CLOSE

#endif // EQ_STDEXT_VC8
#endif // COBASE_UINT128_H
