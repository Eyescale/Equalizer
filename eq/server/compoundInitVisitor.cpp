
/* Copyright (c) 2007-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Daniel Nachbaur <danielnachbaur@gmail.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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
#include <eq/client/log.h>

namespace eq
{
namespace server
{
CompoundInitVisitor::CompoundInitVisitor( )
        : _taskID( 0 )
{}

VisitorResult CompoundInitVisitor::visit( Compound* compound )
{
    Channel* channel = compound->getChannel();

    compound->setTaskID( ++_taskID );
    if( channel && channel->getView( ))
        channel->getView()->updateFrusta();
    else
        compound->updateFrustum( Vector3f::ZERO, 1.f );

    compound->updateInheritData( 0 ); // Compound::activate needs _inherit.eyes

    if( !channel || // non-channel root compounds
        ( compound->isDestination() && !channel->getSegment( )))
    {
        // Note: The second case are non-view destination compounds. One use
        // case is swap-syncing all output channels using task-less compounds.

        LBASSERT( !channel || !channel->getView( ));
        uint32_t eyes = compound->getEyes();
        if( eyes == fabric::EYE_UNDEFINED )
            eyes = fabric::EYES_ALL;

        compound->activate( eyes );
    }

    if( channel )
    {
        compound->updateInheritTasks();
        channel->addTasks( compound->getInheritTasks( ));
    }
    return TRAVERSE_CONTINUE;
}

}
}
