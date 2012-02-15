
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef COBASE_ERRORREGISTRY_H
#define COBASE_ERRORREGISTRY_H

#include <co/base/api.h>
#include <co/base/nonCopyable.h> // base class
#include <co/base/types.h>

namespace co 
{
namespace base
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
     * @sa co::base::Error, eq::Error
     */
    class ErrorRegistry : public NonCopyable
    {
    public:
        /** @internal Construct an error registry. */
        ErrorRegistry();

        ~ErrorRegistry(); //!< @internal

        /** @return the error string for the given error code. @version 1.0 */
        COBASE_API const std::string& getString( const uint32_t error ) const;

        /** Set an error string for the given error code. @version 1.0 */
        COBASE_API void setString( const uint32_t error,
                                   const std::string& text );

        /** Clear a given error code string. @version 1.0 */
        COBASE_API void eraseString( const uint32_t error );

        COBASE_API bool isEmpty() const; //!< @internal

    private:
        detail::ErrorRegistry* const _impl;
    };
}
}
#endif // COBASE_ERRORREGISTRY_H
