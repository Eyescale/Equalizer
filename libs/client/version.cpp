
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "version.h"
#include <sstream>

using namespace eq;

uint32_t Version::getMajor() 
{
    return EQ_VERSION_MAJOR; 
}
uint32_t Version::getMinor()
{
    return EQ_VERSION_MINOR; 
}
uint32_t Version::getPatch() 
{
    return EQ_VERSION_PATCH; 
}

uint32_t Version::getInt()
{
    return ( EQ_VERSION_MAJOR * 10000 +
             EQ_VERSION_MINOR * 100   +
             EQ_VERSION_PATCH ); 
}
float Version::getFloat() 
{
    return ( EQ_VERSION_MAJOR +
             .01f   * EQ_VERSION_MINOR   +
             .0001f * EQ_VERSION_PATCH ); 
}
std::string Version::getString()
{
    std::ostringstream  version;
    version << EQ_VERSION_MAJOR << '.' << EQ_VERSION_MINOR;
    if( EQ_VERSION_PATCH > 0 )
        version << '.' << EQ_VERSION_PATCH;

    return version.str();
}
