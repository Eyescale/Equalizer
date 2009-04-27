
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

#include "compoundUpdateDataVisitor.h"

#include "compound.h"

#include <eq/client/client.h>
#include <eq/client/log.h>
#include <eq/client/server.h>

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

#if 0
    View& view = compound->getView();
    if( view.getID() != EQ_ID_INVALID )
        view.sync();
#endif

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
