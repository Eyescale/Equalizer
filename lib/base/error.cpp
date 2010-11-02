
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

#include "error.h"

#include <eq/base/errorRegistry.h>
#include <eq/base/global.h>

#include <iostream>

namespace eq
{
namespace base
{
std::ostream& operator << ( std::ostream& os, const Error& error )
{
    const base::ErrorRegistry& registry = base::Global::getErrorRegistry();
    const std::string& text = registry.getString( error );
    if( text.empty( ))
        os << "error " << uint32_t( error );
    else
        os << text << " (" << uint32_t( error ) << ")";

    return os;
}

}
}
