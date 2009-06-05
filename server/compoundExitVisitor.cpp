
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
