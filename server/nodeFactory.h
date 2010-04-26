
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQSERVER_NODEFACTORY_H
#define EQSERVER_NODEFACTORY_H

#include "canvas.h"
#include "channel.h"
#include "layout.h"
#include "node.h"
#include "observer.h"
#include "pipe.h"
#include "segment.h"
#include "view.h"
#include "window.h"

namespace eq
{
namespace server
{
    class NodeFactory
    {
    public:
        Config* createConfig( ServerPtr parent ) { return new Config( parent );}
        void releaseConfig( Config* config ) { delete config; }

        Node* createNode( Config* parent ) { return new Node( parent ); }
        void releaseNode( Node* node ) { delete node; }

        Observer* createObserver( Config* parent ){return new Observer(parent);}
        void releaseObserver( Observer* observer ) { delete observer; }

        Layout* createLayout( Config* parent ) { return new Layout( parent ); }
        void releaseLayout( Layout* layout ) { delete layout; }

        View* createView( Layout* parent ) { return new View( parent ); }
        void releaseView( View* view ) { delete view; }

        Canvas* createCanvas( Config* parent ) { return new Canvas( parent ); }
        void releaseCanvas( Canvas* canvas ) { delete canvas; }

        Segment* createSegment( Canvas* parent ) { return new Segment(parent); }
        void releaseSegment( Segment* segment ) { delete segment; }

        Pipe* createPipe( Node* parent ) { return new Pipe( parent ); }
        void releasePipe( Pipe* pipe ) { delete pipe; }

        Window* createWindow( Pipe* parent ) { return new Window( parent ); }
        void releaseWindow( Window* window ) { delete window; }

        Channel* createChannel( Window* parent ) { return new Channel(parent); }
        void releaseChannel( Channel* channel ) { delete channel; }

        ~NodeFactory(){}
    };
}
}

#endif // EQSERVER_NODEFACTORY_H

