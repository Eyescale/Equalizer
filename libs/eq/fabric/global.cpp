
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

#include "global.h"

#include "errorRegistry.h"

namespace eq
{
namespace fabric
{
namespace
{
static std::string _server;
static ErrorRegistry _errorRegistry;
}

void Global::setServer( const std::string& server )
{
    _server = server;
}

const std::string& Global::getServer()
{
    return _server;
}

ErrorRegistry& Global::getErrorRegistry()
{
    return _errorRegistry;
}

}
}
