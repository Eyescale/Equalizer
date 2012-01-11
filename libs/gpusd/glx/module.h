
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef GPUSD_GLX_MODULE_H
#define GPUSD_GLX_MODULE_H

#include <gpusd/module.h> // base class

namespace gpusd
{
namespace glx
{
    /**
     * The GLX implementation for local GPU discovery.
     * 
     * The GLX module tries to open X server :0 to :9. For each server which can
     * be opened, all available screens on the server are added as a separate
     * GPUInfo.
     */
    class Module : public gpusd::Module
    {
    public:
        /** Instantiate an AGL discovery module for the process. */
        static GPUSD_API void use();

    protected:
        virtual GPUInfos discoverGPUs_() const;

    private:
        Module() : gpusd::Module() {}
        virtual ~Module() {}
    };
}
}
#endif // GPUSD_GLX_MODULE_H

