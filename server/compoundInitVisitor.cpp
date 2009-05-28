
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "compoundInitVisitor.h"

#include "config.h"
#include "frame.h"
#include "log.h"
#include "segment.h"
#include "view.h"
#include "window.h"
#include "swapBarrier.h"
#include <eq/client/log.h>

using namespace std;
using namespace eq::base;

namespace eq
{
namespace server
{
CompoundInitVisitor::CompoundInitVisitor()
        : _taskID( 0 )
{}

VisitorResult CompoundInitVisitor::visit( Compound* compound )
{
    compound->setTaskID( ++_taskID );

    Channel* channel = compound->getChannel();
    if( compound->isDestination() && !channel->getSegment( ))
    {
        EQASSERT( !channel->getView( ));
        
        // old-school (non-Layout) destination channel, activate compound
        //  layout destination channel compounds are activated by canvas
        compound->activate();
    }
    
    if( channel )
    {
        const SwapBarrier* swapBarrier = compound->getSwapBarrier();
        if( swapBarrier )
        {
            if( swapBarrier->isNvSwapBarrier( ))
            {
                Window* window = channel->getWindow();
                window->joinNVSwapBarrier( swapBarrier );
            }
        }
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

    compound->updateFrustum();
    compound->updateInheritData( 0 ); // set up initial values

    if( channel )
        channel->addTasks( compound->getInheritTasks( ));

    return TRAVERSE_CONTINUE;    
}

}
}
