
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "swapBarrier.h"

namespace eq
{
namespace fabric
{

std::ostream& operator << ( std::ostream& os, const SwapBarrier& swapBarrier )
{
    if( swapBarrier.isNvSwapBarrier( ))
        return os << lunchbox::disableFlush << "swapbarrier" << std::endl 
                  << "{"<< std::endl 
                  << "    name \"" << swapBarrier.getName() << "\"" << std::endl
                  << "    NV_group " << swapBarrier.getNVSwapGroup() <<std::endl
                  << "    NV_barrier " << swapBarrier.getNVSwapBarrier()
                  << std::endl
                  << "}"  << lunchbox::enableFlush << std::endl; 

    return os << lunchbox::disableFlush << "swapbarrier { name \"" 
              << swapBarrier.getName() << "\" }" << lunchbox::enableFlush
              << std::endl;
}

}
}
