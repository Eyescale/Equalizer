
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compoundExitVisitor.h"

#include "config.h"
#include "frame.h"
#include "swapBarrier.h"
#include "window.h"

#include <eq/client/log.h>

using namespace std;
using namespace eq::base;

namespace eq
{
namespace server
{
CompoundExitVisitor::CompoundExitVisitor()
{}

VisitorResult CompoundExitVisitor::visit( Compound* compound )
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

    const SwapBarrier* swapBarrier = compound->getSwapBarrier();
    if( swapBarrier )
    {
        if( swapBarrier->isNvSwapBarrier( ))
        {
            Window* window = compound->getWindow();
            if( window )
                window->leaveNVSwapBarrier( swapBarrier );
        }
    }

    Channel* channel = compound->getChannel();
    if( compound->isDestination() && !channel->getSegment( ))
    {
        EQASSERT( !channel->getView( ));
        
        // old-school (non-Layout) destination channel, deactivate compound
        //  layout destination channel compounds are deactivated by canvas
        compound->deactivate();
    }
    
    return TRAVERSE_CONTINUE;    
}

}
}
