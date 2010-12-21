
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "compoundUpdateDataVisitor.h"

#include "compound.h"

#include <eq/log.h>

namespace eq
{
namespace server
{
using fabric::TASK_DRAW;

CompoundUpdateDataVisitor::CompoundUpdateDataVisitor(
    const uint32_t frameNumber )
        : _frameNumber( frameNumber )
        , _taskID( 0 )
{}

VisitorResult CompoundUpdateDataVisitor::visit( Compound* compound )
{
    compound->setTaskID( ++_taskID );
    compound->fireUpdatePre( _frameNumber );
    compound->updateInheritData( _frameNumber );

    _updateDrawFinish( compound );
    return TRAVERSE_CONTINUE;    
}


void CompoundUpdateDataVisitor::_updateDrawFinish( Compound* compound )
{
    if( compound->testInheritTask( TASK_DRAW ) && compound->isRunning( ))
    {
        Channel* channel = compound->getChannel();
        channel->setLastDrawCompound( compound );
    }
}

}
}
