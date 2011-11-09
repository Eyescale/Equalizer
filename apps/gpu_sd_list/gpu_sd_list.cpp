
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


#include <gpusd1/gpuInfo.h>
#include <gpusd1/module.h>

int main (int argc, const char * argv[])
{
    const gpusd::GPUInfos gpus = gpusd::Module::discoverGPUs();
    for( gpusd::GPUInfosCIter i = gpus.begin(); i != gpus.end(); ++i )
        std::cout << *i << std::endl;

    return 0;
}
