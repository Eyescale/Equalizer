
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
        os << "swapBarrier" << endl 
           << "{"<< endl 
           << "    name \"" << swapBarrier->getName() << "\"" << endl
           << "    group " << swapBarrier->getNVGroup() << endl
           << "    barrier " << swapBarrier->getNVBarrier()<< endl
           << "}"  << endl; 
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
