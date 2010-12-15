
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "configUpdateDataVisitor.h"

#include "channel.h"
#include "node.h"
#include "pipe.h"
#include "view.h"
#include "window.h"

using namespace std;

namespace eq
{
namespace server
{
ConfigUpdateDataVisitor::ConfigUpdateDataVisitor()
    : _lastDrawChannel( 0 )
    , _lastDrawWindow( 0 )
    , _lastDrawPipe( 0 )
{}


VisitorResult ConfigUpdateDataVisitor::visitPre( Node* node )
{
    _lastDrawPipe = 0;
    return TRAVERSE_CONTINUE;
}
VisitorResult ConfigUpdateDataVisitor::visitPost( Node* node )
{
    node->setLastDrawPipe( _lastDrawPipe );
    return TRAVERSE_CONTINUE;
}

VisitorResult ConfigUpdateDataVisitor::visitPre( Pipe* pipe )
{
    _lastDrawWindow = 0;
    return TRAVERSE_CONTINUE;
}
VisitorResult ConfigUpdateDataVisitor::visitPost( Pipe* pipe )
{
    pipe->setLastDrawWindow( _lastDrawWindow );
    if( _lastDrawWindow )
        _lastDrawPipe = pipe;
    return TRAVERSE_CONTINUE;
}

VisitorResult ConfigUpdateDataVisitor::visitPre( Window* window )
{
    _lastDrawChannel = 0;
    return TRAVERSE_CONTINUE;
}
VisitorResult ConfigUpdateDataVisitor::visitPost( Window* window )
{
    window->setLastDrawChannel( _lastDrawChannel );
    if( _lastDrawChannel )
        _lastDrawWindow = window;
    return TRAVERSE_CONTINUE;
}

VisitorResult ConfigUpdateDataVisitor::visit( Channel* channel )
{
    if( channel->getLastDrawCompound( ))
        _lastDrawChannel = channel;

    return TRAVERSE_CONTINUE;
}
 
}
}
