
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

#include "types.h"

namespace eq
{
namespace server
{
    class NodeFactory
    {
    public:
        Config* createConfig( ServerPtr parent );
        void releaseConfig( Config* config );

        Node* createNode( Config* parent );
        void releaseNode( Node* node );

        Observer* createObserver( Config* parent );
        void releaseObserver( Observer* observer );

        Layout* createLayout( Config* parent );
        void releaseLayout( Layout* layout );

        View* createView( Layout* parent );
        void releaseView( View* view );

        Canvas* createCanvas( Config* parent );
        void releaseCanvas( Canvas* canvas );

        Segment* createSegment( Canvas* parent );
        void releaseSegment( Segment* segment );

        Pipe* createPipe( Node* parent );
        void releasePipe( Pipe* pipe );

        Window* createWindow( Pipe* parent );
        void releaseWindow( Window* window );

        Channel* createChannel( Window* parent );
        void releaseChannel( Channel* channel );

        ~NodeFactory(){}
    };
}
}

#endif // EQSERVER_NODEFACTORY_H

