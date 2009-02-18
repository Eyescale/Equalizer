
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compoundInitVisitor.h"

#include "config.h"
#include "frame.h"
#include "log.h"
#include "segment.h"
#include "view.h"

#include <eq/client/log.h>

using namespace std;
using namespace eq::base;

namespace eq
{
namespace server
{
CompoundInitVisitor::CompoundInitVisitor()
{}

VisitorResult CompoundInitVisitor::visit( Compound* compound )
{
    Channel* channel = compound->getChannel();
    if( channel )
    {
        channel->refUsed();

        const Segment* segment = channel->getSegment();
        if( segment ) // we are a created destination channel
            _updateFrustum( compound );
    }

    Config*        config  = compound->getConfig();
    const uint32_t latency = config->getLatency();
    EQASSERT( config );
    
    const std::vector< Frame* >& outputFrames = compound->getOutputFrames();
    for( vector<Frame*>::const_iterator i = outputFrames.begin(); 
         i != outputFrames.end(); ++i )
    {
        Frame* frame = *i;
        config->registerObject( frame );
        frame->setAutoObsolete( latency );
        EQLOG( eq::LOG_ASSEMBLY ) << "Output frame \"" << frame->getName() 
                                  << "\" id " << frame->getID() << endl;
    }

    const std::vector< Frame* >& inputFrames = compound->getInputFrames();
    for( vector<Frame*>::const_iterator i = inputFrames.begin(); 
         i != inputFrames.end(); ++i )
    {
        Frame* frame = *i;
        config->registerObject( frame );
        frame->setAutoObsolete( latency );
        EQLOG( eq::LOG_ASSEMBLY ) << "Input frame \"" << frame->getName() 
                                  << "\" id " << frame->getID() << endl;
    }

    compound->updateInheritData( 0 ); // set up initial values
    if( channel )
        channel->addTasks( compound->getInheritTasks( ));

    return TRAVERSE_CONTINUE;    
}

void CompoundInitVisitor::_updateFrustum( Compound* compound )
{
    const Channel* channel = compound->getChannel();
    const Segment* segment = channel->getSegment();
    EQASSERT( channel && segment );

    switch( segment->getCurrentType( )) // derive our frustum:
    {
        case View::TYPE_WALL:
        {
            // set compound frustum =
            //         segment frustum X channel/segment coverage
            const Channel* outputChannel = segment->getChannel();
            EQASSERT( outputChannel );

            const Viewport& outputVP = outputChannel->getViewport();
            Viewport partialArea( channel->getViewport( ));
            partialArea.intersect( outputVP ); // intersection
            partialArea.transform( outputVP ); // in output coord.

            Wall wall( segment->getWall( ));
            wall.apply( partialArea );
            compound->setWall( wall );
            EQLOG( LOG_VIEW ) << "Compound wall: " << wall << std::endl;
            break;
        }

        case View::TYPE_PROJECTION:
            EQUNIMPLEMENTED;
            break;

        default: 
            break;
    }

}

}
}
