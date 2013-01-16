
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "nodeFactory.h"

#include "canvas.h"
#include "channel.h"
#include "client.h"
#include "config.h"
#include "layout.h"
#include "node.h"
#include "observer.h"
#include "pipe.h"
#include "segment.h"
#include "server.h"
#include "view.h"
#include "window.h"

namespace eq
{

Config* NodeFactory::createConfig( ServerPtr parent )
{
    return new Config( parent );
}
void NodeFactory::releaseConfig( Config* config )
{
    delete config;
}

Node* NodeFactory::createNode( Config* parent )
{
    return new Node( parent );
}
void NodeFactory::releaseNode( Node* node )
{
    delete node;
}

Observer* NodeFactory::createObserver( Config* parent )
{
    return new Observer( parent );
}
void NodeFactory::releaseObserver( Observer* observer )
{
    delete observer;
}

Layout* NodeFactory::createLayout( Config* parent )
{
    return new Layout( parent );
}
void NodeFactory::releaseLayout( Layout* layout )
{
    delete layout;
}

View* NodeFactory::createView( Layout* parent )
{
    return new View( parent );
}
void NodeFactory::releaseView( View* view )
{
    delete view;
}

Canvas* NodeFactory::createCanvas( Config* parent )
{
    return new Canvas( parent );
}
void NodeFactory::releaseCanvas( Canvas* canvas )
{
    delete canvas;
}

Segment* NodeFactory::createSegment( Canvas* parent )
{
    return new Segment( parent );
}
void NodeFactory::releaseSegment( Segment* segment )
{
    delete segment;
}

Pipe* NodeFactory::createPipe( Node* parent )
{
    return new Pipe( parent );
}
void NodeFactory::releasePipe( Pipe* pipe )
{
    delete pipe;
}

Window* NodeFactory::createWindow( Pipe* parent )
{
    return new Window( parent );
}
void NodeFactory::releaseWindow( Window* window )
{
    delete window;
}

Channel* NodeFactory::createChannel( Window* parent )
{
    return new Channel( parent );
}

void NodeFactory::releaseChannel( Channel* channel )
{
    delete channel;
}

}

