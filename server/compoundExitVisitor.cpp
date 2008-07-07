
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compoundExitVisitor.h"

#include "frame.h"

#include <eq/client/log.h>

using namespace std;
using namespace eq::base;

namespace eqs
{
CompoundExitVisitor::CompoundExitVisitor()
{}

Compound::VisitorResult CompoundExitVisitor::visitLeaf( Compound* compound )
{
    Config* config = compound->getConfig();
    EQASSERT( config );

    const std::vector< Frame* >& outputFrames = compound->getOutputFrames();
    for( vector<Frame*>::const_iterator i = outputFrames.begin(); 
         i != outputFrames.end(); ++i )
    {
        Frame* frame = *i;
        frame->flush();
        config->deregisterObject( frame );
    }

    const std::vector< Frame* >& inputFrames = compound->getInputFrames();
    for( vector<Frame*>::const_iterator i = inputFrames.begin(); 
         i != inputFrames.end(); ++i )
    {
        Frame* frame = *i;
        config->deregisterObject( frame );
    }

    Channel* channel = compound->getChannel();
    if( channel )
        channel->unrefUsed();

    return Compound::TRAVERSE_CONTINUE;    
}

}

