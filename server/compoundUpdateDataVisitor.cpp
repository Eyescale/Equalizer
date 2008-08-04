
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compoundUpdateDataVisitor.h"

#include <eq/client/log.h>

using namespace std;
using namespace eq::base;

namespace eq
{
namespace server
{
CompoundUpdateDataVisitor::CompoundUpdateDataVisitor(
    const uint32_t frameNumber )
        : _frameNumber( frameNumber )
{}

Compound::VisitorResult CompoundUpdateDataVisitor::visitLeaf(
    Compound* compound )
{
    compound->fireUpdatePre( _frameNumber );
    compound->updateInheritData( _frameNumber );
    _updateDrawFinish( compound );
    return Compound::TRAVERSE_CONTINUE;    
}


void CompoundUpdateDataVisitor::_updateDrawFinish( Compound* compound )
{
    if( !compound->testInheritTask( Compound::TASK_DRAW ))
        return;

    Channel* channel = compound->getChannel();
    channel->setLastDrawCompound( compound );
}

}
}
