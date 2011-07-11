
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.h> 
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

#include "display.h"

#include "../canvas.h"
#include "../channel.h"
#include "../config.h"
#include "../layout.h"
#include "../node.h"
#include "../observer.h"
#include "../segment.h"
#include "../view.h"
#include "../window.h"

namespace eq
{
namespace server
{
namespace config
{

void Display::discoverLocal( Config* config )
{
    Node* node = config->findAppNode();
    EQASSERT( node );
    if( !node )
        return;

    const Pipes& pipes = node->getPipes();
    EQASSERT( !pipes.empty( ));
    if( pipes.empty( ))
        return;

    Pipe* pipe = pipes.front();
    Window* window = new Window( pipe );
    window->setViewport( Viewport( .25f, .2f, .5f, .5f ));

    Channel* channel = new Channel( window );
    new Observer( config );
    Layout* layout = new Layout( config );
    new View( layout );
    Canvas* canvas = new Canvas( config );
    Segment* segment = new Segment( canvas );

    canvas->addLayout( layout );
    segment->setChannel( channel );
}

}
}
}
