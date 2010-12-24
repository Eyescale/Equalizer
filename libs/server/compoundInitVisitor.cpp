
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010,      Cedric Stalder <cedric.stalder@gmail.com>
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
#include <eq/log.h>

using namespace std;
using namespace co::base;

namespace eq
{
namespace server
{
CompoundInitVisitor::CompoundInitVisitor( )
        : _taskID( 0 )
{}

VisitorResult CompoundInitVisitor::visit( Compound* compound )
{
    compound->setTaskID( ++_taskID );
    compound->updateFrustum();
    compound->updateInheritData( 0 ); // Compound::activate needs _inherit.eyes

    Channel* channel = compound->getChannel();
    if( !channel || // non-channel root compounds
        ( compound->isDestination() && !channel->getSegment( )))
    {
        // Note: The second case are non-view destination compounds. One use
        // case are swap-syncing all output channels using task-less compounds.

        EQASSERT( !channel || !channel->getView( ));
        uint32_t eyes = compound->getEyes();
        if( eyes == fabric::EYE_UNDEFINED )
            eyes = fabric::EYES_ALL;

        compound->activate( eyes );
    }

    if( channel )
    {
        compound->initInheritTasks();
        channel->addTasks( compound->getInheritTasks( ));
    }
    return TRAVERSE_CONTINUE;    
}

}
}
