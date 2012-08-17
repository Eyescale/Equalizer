
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include <co/version.h>
#include <sstream>

#define QUOTE( string ) STRINGIFY( string )
#define STRINGIFY( foo ) #foo

namespace co
{

uint32_t Version::getMajor() 
{
    return CO_VERSION_MAJOR; 
}
uint32_t Version::getMinor()
{
    return CO_VERSION_MINOR; 
}
uint32_t Version::getPatch() 
{
    return CO_VERSION_PATCH; 
}
std::string Version::getRevision() 
{
    return std::string( QUOTE( CO_VERSION_REVISION ));
}
uint32_t Version::getABI() 
{
    return CO_VERSION_ABI; 
}

uint32_t Version::getInt()
{
    return ( CO_VERSION_MAJOR * 10000 +
             CO_VERSION_MINOR * 100   +
             CO_VERSION_PATCH ); 
}
float Version::getFloat() 
{
    return ( CO_VERSION_MAJOR +
             .01f   * CO_VERSION_MINOR   +
             .0001f * CO_VERSION_PATCH ); 
}
std::string Version::getString()
{
    std::ostringstream  version;
    version << CO_VERSION_MAJOR << '.' << CO_VERSION_MINOR << '.'
            << CO_VERSION_PATCH;

    const std::string revision = getRevision();
    if( revision != "0" )
        version << '.' << revision;

    return version.str();
}

}
