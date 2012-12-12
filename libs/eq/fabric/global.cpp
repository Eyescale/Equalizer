
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@eyescale.ch>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "configParams.h"
#include "errorRegistry.h"

namespace eq
{
namespace fabric
{
namespace
{
static std::string _server;
static ErrorRegistry _errorRegistry;
static uint32_t _flags = ConfigParams::FLAG_NONE;
static Strings _prefixes;
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

void Global::setFlags( const uint32_t flags )
{
    _flags = flags;
}

uint32_t Global::getFlags()
{
    return _flags;
}

void Global::setPrefixes( const Strings& prefixes )
{
    _prefixes = prefixes;
}

const Strings& Global::getPrefixes()
{
    return _prefixes;
}


}
}
