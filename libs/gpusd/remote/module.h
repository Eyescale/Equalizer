
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

#ifndef GPUSD_REMOTE_MODULE_H
#define GPUSD_REMOTE_MODULE_H

#include <gpusd1/remote/api.h>
#include <gpusd1/types.h>
#include <iostream>

namespace gpusd
{
namespace remote
{
    /** Base class for runtime-attached DSOs containing a query implementation.*/
    class Module
    {
    public:
        /** Register and construct a new module. */
        GPUSD_REMOTE_API Module();

        /** Destruct and destruct a module. */
        GPUSD_REMOTE_API virtual ~Module();

        /** @return information about all found GPUs. */
        static GPUInfos discoverGPUs();

    protected:
        /** @return information about all found GPUs. */
        virtual GPUInfos discoverGPUs_() const = 0;
        
    private:
        Module* next_;
    };
}
}
#endif // GPUSD_REMOTE_MODULE_H

