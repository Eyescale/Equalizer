
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef EQBASE_UINT128_H
#define EQBASE_UINT128_H

#include <eq/base/base.h>
#include <eq/base/hash.h>

namespace eq
{
namespace base
{

    /** A base type for 128 bit unsigned integer values. */
    class EQ_BASE_DECL uint128_t
    {
    public:
        /**
         * Construct a new 128 bit integer with a default value.
         * @version 1.0
         */
        uint128_t( const uint64_t low = 0 ) 
            : _high( 0 ), _low( low ) {}

        /**
         * Construct a new 128 bit integer with default values.
         * @version 1.0
         **/
        uint128_t( const uint64_t high, const uint64_t low ) 
            : _high( high ), _low( low ) {}

        /** Assign another 128 bit value. @version 1.0 */
        uint128_t& operator = ( const uint128_t& rhs )
            {
                _high = rhs.getHigh();
                _low = rhs.getLow();
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

        /** @internal @return the lower 64 bits of this UINT128. */
        uint64_t getLow() const { return _low; }
        /** @internal @return the higher 64 bits of this UINT128. */
        uint64_t getHigh() const { return _high; }

        /** @internal @return the lower 64 bits of this UINT128. */
        uint64_t setLow( const uint128_t low ) const { return _low; }
        /** @internal @return the higher 64 bits of this UINT128. */
        uint64_t setHigh( const uint128_t high ) const { return _high; }
    
protected:
#ifndef _MSC_VER
        friend struct stde::hash< eq::base::UUID >;
#endif
    private:
        uint64_t _high;
        uint64_t _low;

        friend class UUID;

        friend uint128_t operator+ ( const uint128_t& a, const uint64_t& b );
        friend uint128_t operator- ( const uint128_t& a, const uint64_t& b );
        
    };

    /** A hash for uint128t_t keys. @version 1.0 */
    template<class T> class uint128_tHash 
                     : public stde::hash_map< uint128_t, T > {};

    /** ostream operator for 128 bit unsigned integers. @version 1.0 */
    inline std::ostream& operator << ( std::ostream& os, const uint128_t& id )
    {
        os << std::hex << id.getHigh() << ':' << id.getLow() << std::dec;
        return os;
    }

    /** Add a 64 bit value to a 128 bit value. @version 1.0 */
    inline uint128_t operator+ ( const uint128_t& a, const uint64_t& b ) 
    {
        uint128_t result = a;
        result._low += b;
        if( result._low < a._low )
            ++result._high;
        return result;
    };

    /** Substract a 64 bit value from a 128 bit value. @version 1.0 */
    inline uint128_t operator- ( const uint128_t& a, const uint64_t& b ) 
    {
        uint128_t result = a;
        result._low -= b;
        if( result._low > a._low )
            --result._high;
        return result;
    };

}
}
#ifdef EQ_STDEXT_VC8
    template<> inline size_t stde::hash_compare< eq::base::uint128_t >::operator() 
        ( const eq::base::uint128_t& key ) const
    {
        return key.getLow();
    }

    template<> inline size_t stde::hash_value( const eq::base::uint128_t& key )
    {
        return key.getLow();
    }

    #else

    EQ_STDEXT_NAMESPACE_OPEN
        template<> struct hash< eq::base::UUID >
        {
            size_t operator()( const eq::base::UUID& key ) const
            {
                return key._high ^ key._low;
            }
        };
    EQ_STDEXT_NAMESPACE_CLOSE

#endif
#endif // EQBASE_UINT128_H
