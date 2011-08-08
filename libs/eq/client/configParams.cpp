
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <co/global.h>

namespace eq
{
ConfigParams::ConfigParams()
        : _renderClient( co::Global::getProgramName( ))
        , _workDir( co::Global::getWorkDir( ))
{
}

void ConfigParams::setRenderClient( const std::string& renderClient )
{
    _renderClient = renderClient;
}

const std::string& ConfigParams::getRenderClient() const
{
    return _renderClient;
}

void ConfigParams::setWorkDir( const std::string& workDir )
{
    _workDir = workDir;
}

const std::string& ConfigParams::getWorkDir() const
{
    return _workDir;
}

}
