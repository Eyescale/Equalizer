
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "swapBarrier.h"

using namespace std;
using namespace eq::base;

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
        os << disableFlush << "swapBarrier" << endl 
           << "{"<< endl 
           << "    name \"" << swapBarrier->getName() << "\"" << endl
           << "    group " << swapBarrier->getNVSwapGroup() << endl
           << "    barrier " << swapBarrier->getNVSwapBarrier()<< endl
           << "}"  << enableFlush << endl; 
    }
    else
    {
        os << disableFlush << "swapBarrier { name \"" 
           << swapBarrier->getName() << "\" }" << enableFlush << endl;
    }

    return os;
}

}
}
