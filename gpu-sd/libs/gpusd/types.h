
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

#ifndef GPUSD_TYPES_H
#define GPUSD_TYPES_H

#include <vector>
#ifdef _MSC_VER
#  include <memory>
#else
#  include <tr1/memory>
#endif

namespace gpusd
{

struct GPUInfo;
typedef std::vector< GPUInfo > GPUInfos; //!< A vector of GPUInfo structs
typedef GPUInfos::iterator GPUInfosIter; //!< An iterator over GPUInfos
/** A const iterator over GPUInfos. */
typedef GPUInfos::const_iterator GPUInfosCIter;

class Filter;
typedef std::tr1::shared_ptr< Filter > FilterPtr; //!< A Filter shared_ptr 
}

#endif // GPUSD_TYPES_H

