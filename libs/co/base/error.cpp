
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include "error.h"

#include <co/base/errorRegistry.h>
#include <co/base/global.h>

#include <iostream>

namespace co
{
namespace base
{
std::ostream& operator << ( std::ostream& os, const Error& error )
{
    const ErrorRegistry& registry = Global::getErrorRegistry();
    const std::string& text = registry.getString( error );
    if( text.empty( ))
        os << "error 0x" << std::hex << uint32_t( error ) << std::dec;
    else
        os << text << " (0x" << std::hex << uint32_t(error) << std::dec << ")";

    return os;
}

}
}
