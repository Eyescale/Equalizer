
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

using namespace std;
using namespace co::base;

namespace eq
{
namespace server
{

ostream& operator << ( ostream& os, const SwapBarrier* swapBarrier )
{
    if( !swapBarrier )
        return os;
    
    if ( swapBarrier->isNvSwapBarrier() )
    {
        os << disableFlush << "swapbarrier" << endl 
           << "{"<< endl 
           << "    name \"" << swapBarrier->getName() << "\"" << endl
           << "    group " << swapBarrier->getNVSwapGroup() << endl
           << "    barrier " << swapBarrier->getNVSwapBarrier()<< endl
           << "}"  << enableFlush << endl; 
    }
    else
    {
        os << disableFlush << "swapbarrier { name \"" 
           << swapBarrier->getName() << "\" }" << enableFlush << endl;
    }

    return os;
}

}
}
