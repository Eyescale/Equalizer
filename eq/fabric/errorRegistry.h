
/* Copyright (c) 2010-2015, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQFABRIC_ERRORREGISTRY_H
#define EQFABRIC_ERRORREGISTRY_H

#include <eq/fabric/api.h>
#include <eq/fabric/types.h>

namespace eq
{
namespace fabric
{
namespace detail { class ErrorRegistry; }

/**
 * The registry translating error codes to strings.
 *
 * Applications can register custom error strings starting at
 * eq::ERROR_CUSTOM. Error registration and erasure is not
 * thread-safe. Equalizer registers errors only during eq::init(). It is
 * strongly advised to register application-specific errors before
 * eq::init() and erase them after eq::exit().
 *
 * @sa co::Error, eq::Error
 */
class ErrorRegistry
{
public:
    /** @internal Construct an error registry. */
    ErrorRegistry();

    ~ErrorRegistry(); //!< @internal

    /** @return the error string for the given error code. @version 1.0 */
    EQFABRIC_API const std::string& getString( const uint32_t error ) const;

    /** Set an error string for the given error code. @version 1.0 */
    EQFABRIC_API void setString( const uint32_t error,
                                 const std::string& text );

    /** Clear a given error code string. @version 1.0 */
    EQFABRIC_API void eraseString( const uint32_t error );

    EQFABRIC_API bool isEmpty() const; //!< @internal

private:
    ErrorRegistry( const ErrorRegistry& ) = delete;
    ErrorRegistry& operator=( const ErrorRegistry& ) = delete;
    detail::ErrorRegistry* const _impl;
};

}
}
#endif // EQFABRIC_ERRORREGISTRY_H
