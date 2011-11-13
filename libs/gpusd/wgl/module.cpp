
/* Copyright (c) 2011, Daniel Nachbaur <danielnachbaur@gmail.com> 
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

#include "module.h"

#include <gpusd/gpuInfo.h>


namespace gpusd
{
namespace wgl
{
namespace
{

Module* instance = 0;

}

void Module::use()
{
    if( !instance )
        instance = new Module;
}

GPUInfos Module::discoverGPUs_() const
{
    GPUInfos result;
    GPUInfo defaultInfo( "WGL" );
    return result;
}

}
}
