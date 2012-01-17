
/*
  Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>

  This file is part of the GPU-SD discovery tool.

  The GPU-SD discovery tool is free software: you can redistribute it and/or
  modify it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  GPU-SD is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with GPU-SD. If not, see <http://www.gnu.org/licenses/>.
*/


#include <gpusd/gpuInfo.h>
#include <gpusd/dns_sd/module.h>

int main (int argc, const char * argv[])
{
    gpusd::dns_sd::Module::use();
    const gpusd::GPUInfos gpus = gpusd::Module::discoverGPUs();
    for( gpusd::GPUInfosCIter i = gpus.begin(); i != gpus.end(); ++i )
        std::cout << *i << std::endl;

    return 0;
}
