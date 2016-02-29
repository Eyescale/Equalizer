
/* Copyright (c) 2010-2016, Stefan Eilemann <eile@eyescale.ch>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQFABRIC_ERROR_H
#define EQFABRIC_ERROR_H

#include <eq/fabric/api.h>
#include <eq/fabric/types.h>
#include <lunchbox/bitOperation.h> // inline template specialization

namespace eq
{
namespace fabric
{

/** A wrapper for error codes to allow intuitive bool-like usage. */
class Error
{
    typedef void (Error::*bool_t)() const;
    void bool_true() const {}

public:
    /** Construct a new error. @version 1.7.1 */
    EQFABRIC_API Error( const uint32_t code,
                        const uint128_t& originator = uint128_t( ));

    /** Assign the given error code. @version 1.7.1*/
    EQFABRIC_API Error& operator = ( const ErrorCode code );

    /** @return true if an error occured. @version 1.7.1 */
    EQFABRIC_API operator bool_t() const;

    /** @return true if no error occured. @version 1.7.1 */
    EQFABRIC_API bool operator ! () const;

    /** @return the error code. @version 1.7.1 */
    EQFABRIC_API uint32_t getCode() const;

    /** @return the ID of the originator, a co::Object. @version 1.9 */
    EQFABRIC_API const uint128_t& getOriginator() const;

    /** @return true if the two errors have the same value. @version 1.7.1*/
    EQFABRIC_API bool operator == ( const Error& rhs ) const;

    /** @return true if the two errors have different values. @version 1.7.1*/
    EQFABRIC_API bool operator != ( const Error& rhs ) const;

    /** @return true if the two errors have the same value. @version 1.7.1*/
    EQFABRIC_API bool operator == ( const uint32_t code ) const;

    /** @return true if the two errors have different values. @version 1.7.1*/
    EQFABRIC_API bool operator != ( const uint32_t code ) const;

    EQFABRIC_API Error(); //!< @internal
    EQFABRIC_API void serialize( co::DataOStream& os ) const; //!< @internal
    EQFABRIC_API void deserialize( co::DataIStream& is ); //!< @internal

private:
    friend void lunchbox::byteswap< Error >( Error& value );

    uint32_t _code;
    uint128_t _originator;
};

/** Print the error in a human-readable format. @version 1.0 */
EQFABRIC_API std::ostream& operator << ( std::ostream& os, const Error& );

}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::Error& value )
{
    byteswap( value._code );
    byteswap( value._originator );
}
}

#endif // EQFABRIC_ERROR_H
