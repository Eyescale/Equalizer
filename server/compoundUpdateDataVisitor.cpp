
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compoundUpdateDataVisitor.h"

#include "compound.h"

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

VisitorResult CompoundUpdateDataVisitor::visit(
    Compound* compound )
{
    compound->fireUpdatePre( _frameNumber );

    View& view = compound->getView();
    if( view.getID() != EQ_ID_INVALID )
        view.sync();

    compound->updateInheritData( _frameNumber );
    _updateDrawFinish( compound );
    return TRAVERSE_CONTINUE;    
}


void CompoundUpdateDataVisitor::_updateDrawFinish( Compound* compound )
{
    if( !compound->testInheritTask( eq::TASK_DRAW ) ||
        !compound->isActive( ))
        return;

    Channel* channel = compound->getChannel();
    channel->setLastDrawCompound( compound );
}

}
}
