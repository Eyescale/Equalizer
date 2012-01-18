
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

#include <gpusd/version.h>
#include <sstream>

namespace gpusd
{

int Version::getMajor() 
{
    return GPUSD_VERSION_MAJOR; 
}

int Version::getMinor()
{
    return GPUSD_VERSION_MINOR; 
}

int Version::getPatch() 
{
    return GPUSD_VERSION_PATCH; 
}

std::string Version::getString()
{
    std::ostringstream version;
    version << GPUSD_VERSION_MAJOR << '.' << GPUSD_VERSION_MINOR << '.'
            << GPUSD_VERSION_PATCH;
    return version.str();
}

}
