
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

#ifndef GPUSD_MODULE_H
#define GPUSD_MODULE_H

#include <gpusd/api.h>
#include <gpusd/filter.h>
#include <gpusd/types.h>
#include <iostream>


/**
 * @namespace gpusd
 * @brief The GPU-SD discovery library
 */
namespace gpusd
{
namespace detail
{
    class Module;
}
    /** Base class for runtime-attached DSOs of a query implementation. */
    class Module
    {
    public:
        /** Register and construct a new module. @version 1.0 */
        GPUSD_API Module();

        /** Destruct this module. @version 1.0 */
        GPUSD_API virtual ~Module();

        /** @return information about all found GPUs. @version 1.0 */
        GPUSD_API static GPUInfos discoverGPUs( FilterPtr filter =
                                                FilterPtr(new DuplicateFilter));
    protected:
        /** @return information about all found GPUs. @version 1.0 */
        virtual GPUInfos discoverGPUs_() const = 0;
        
    private:
        detail::Module* const impl_;
    };
}
#endif // GPUSD_MODULE_H

