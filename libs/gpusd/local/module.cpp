
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

#include "module.h"

#include <gpusd1/local/gpuInfo.h>

namespace gpusd
{
namespace local
{
namespace
{
Module* stack_ = 0;
}

Module::Module()
        : next_( stack_ )
{
    stack_ = this;
}

Module::~Module()
{
    Module* previous = stack_;

    for( Module* module = stack_; module; module = module->next_ )
    {
        if( module == this )
        {
            if( previous )
                previous->next_ = next_;
            else
                stack_ = next_;
            return;
        }
        previous = module;
    }
}

GPUInfos Module::discoverGPUs()
{
    for( Module* module = stack_; module; module = module->next_ )
    {
        const GPUInfos result = module->discoverGPUs_();
        if( !result.empty( ))
            return result;
    }

    GPUInfos result;
    return result;
}

}
}
