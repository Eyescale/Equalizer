
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

#include "errorRegistry.h"

#include "debug.h"
#include "error.h"
#include "stdExt.h"

namespace co 
{
namespace base
{
namespace
{
static std::string _empty;
typedef stde::hash_map< uint32_t, std::string > ErrorHash;
}

namespace detail
{
class ErrorRegistry
{
public:
    ErrorHash errors;
};
}

ErrorRegistry::ErrorRegistry()
        : _impl( new detail::ErrorRegistry )
{
    _impl->errors[ ERROR_NONE ] = "no error";
}

ErrorRegistry::~ErrorRegistry()
{
    delete _impl;
}

const std::string& ErrorRegistry::getString( const uint32_t error ) const
{
    ErrorHash::const_iterator i = _impl->errors.find( error );
    if( i == _impl->errors.end( ))
        return _empty;

    return i->second;
}


void ErrorRegistry::setString( const uint32_t error, const std::string& text )
{
    _impl->errors[ error ] = text;
}

void ErrorRegistry::eraseString( const uint32_t error )
{
    _impl->errors.erase( error );
}

bool ErrorRegistry::isEmpty() const
{
    return _impl->errors.empty();
}

}
}
