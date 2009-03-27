
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include <eq/net/global.h>

using namespace std;

namespace eq
{
ConfigParams::ConfigParams()
{
    _renderClient = net::Global::getProgramName();
    _workDir      = net::Global::getWorkDir();

//     compoundModes = 
//         COMPOUND_MODE_2D    | 
//         COMPOUND_MODE_DPLEX |
//         COMPOUND_MODE_EYE   |
//         COMPOUND_MODE_FSAA;
}

ConfigParams& ConfigParams::operator = ( const ConfigParams& rhs )
{
    if( this == &rhs )
        return *this;
 
    _renderClient  = rhs._renderClient;
    _workDir       = rhs._workDir;
//    compoundModes = rhs.compoundModes;
    return *this;
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
