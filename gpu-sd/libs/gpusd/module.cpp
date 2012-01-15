
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

#include <gpusd/module.h>
#include <gpusd/gpuInfo.h>

namespace gpusd
{
namespace
{
Module* stack_ = 0;
}
namespace detail
{
class Module
{
public:
    Module() : next_( 0 ) {}
    ~Module() {};

    gpusd::Module* next_;
};
}

Module::Module()
        : impl_( new detail::Module( ))
{
    if( !stack_ )
    {
        stack_ = this;
        return;
    }

    for( Module* module = stack_; module; module = module->impl_->next_ )
    {
        if( !module->impl_->next_ )
        {
            module->impl_->next_ = this;
            return;
        }
    }
}

Module::~Module()
{
    Module* previous = stack_;
    for( Module* module = stack_; module; module = module->impl_->next_ )
    {
        if( module == this )
        {
            if( previous )
                previous->impl_->next_ = impl_->next_;
            else
                stack_ = impl_->next_;
            return;
        }
        previous = module;
    }
    delete impl_;
}

GPUInfos Module::discoverGPUs( FilterPtr filter )
{
    GPUInfos result;
    for( Module* module = stack_; module; module = module->impl_->next_ )
    {
        const GPUInfos infos = module->discoverGPUs_(); 
        for( GPUInfosCIter i = infos.begin(); i != infos.end(); ++i )
        {
            const GPUInfo& info = *i;
            if( !filter || (*filter)( result, info ))
                result.push_back( info );
        }
    }
    return result;
}

}
