
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "configParams.h"

#include "global.h"
#include <co/global.h>

namespace eq
{
namespace detail
{
class ConfigParams
{
public:
    ConfigParams()
            : renderClient( co::Global::getProgramName( ))
            , workDir( co::Global::getWorkDir( ))
            , flags( eq::Global::getFlags( ))
        {}

    std::string renderClient;
    std::string workDir;
    uint32_t flags;
};
}

ConfigParams::ConfigParams()
        : _impl( new detail::ConfigParams )
{
}

ConfigParams::~ConfigParams()
{
    delete _impl;
}

void ConfigParams::setRenderClient( const std::string& renderClient )
{
    _impl->renderClient = renderClient;
}

const std::string& ConfigParams::getRenderClient() const
{
    return _impl->renderClient;
}

void ConfigParams::setWorkDir( const std::string& workDir )
{
    _impl->workDir = workDir;
}

const std::string& ConfigParams::getWorkDir() const
{
    return _impl->workDir;
}

void ConfigParams::setFlags( const uint32_t flags )
{
    _impl->flags = flags;
}

uint32_t ConfigParams::getFlags() const
{
    return _impl->flags;
}

}
