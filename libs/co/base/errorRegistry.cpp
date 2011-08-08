
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
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

namespace co 
{
namespace base
{
namespace
{
static std::string _empty;
}

const std::string& ErrorRegistry::getString( const uint32_t error ) const
{
    ErrorHash::const_iterator i = _errors.find( error );
    if( i == _errors.end( ))
        return _empty;

    return i->second;
}


void ErrorRegistry::setString( const uint32_t error, const std::string& text )
{
    _errors[ error ] = text;
}

void ErrorRegistry::eraseString( const uint32_t error )
{
    _errors.erase( error );
}

}
}
