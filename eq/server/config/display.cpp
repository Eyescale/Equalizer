
/* Copyright (c) 2011-2013, Stefan Eilemann <eile@eyescale.h>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "resources.h"
#include "../canvas.h"
#include "../channel.h"
#include "../config.h"
#include "../layout.h"
#include "../node.h"
#include "../pipe.h"
#include "../observer.h"
#include "../segment.h"
#include "../view.h"
#include "../window.h"

#include <eq/fabric/configParams.h>

namespace eq
{
namespace server
{
namespace config
{

namespace
{

void _choosePixelViewport( Window& window, const PixelViewport& pvp )
{
    int32_t width = window.getIAttribute( Window::IATTR_HINT_WIDTH );
    int32_t height = window.getIAttribute( Window::IATTR_HINT_HEIGHT );
    if( width == fabric::UNDEFINED && height == fabric::UNDEFINED )
    {
        window.setViewport( Viewport( .25f, .2f, .5f, .5f ));
    }
    else
    {
        if( pvp.isValid( ))
        {
            width = width == fabric::UNDEFINED || width < 0 ?
                pvp.w * .5f : std::min( pvp.w, width );
            height = height == fabric::UNDEFINED || height < 0 ?
                pvp.h * .5f : std::min( pvp.h, height );
        }
        if( width == fabric::UNDEFINED || width < 0)
            /* An arbitray value */
            width = 860;
        if( height == fabric::UNDEFINED || height < 0)
            /* An arbitray value */
            height = 540;
        const uint32_t x = ( pvp.w - width ) * 0.5;
        const uint32_t y = ( pvp.h - height ) * 0.5;

        window.setPixelViewport( PixelViewport( x, y, width, height ));
    }
}

}


void Display::discoverLocal( Config* config, const uint32_t flags )
{
    Node* node = config->findAppNode();
    LBASSERT( node );
    if( !node )
        return;

    const Pipes& pipes = node->getPipes();
    LBASSERT( !pipes.empty( ));
    if( pipes.empty( ))
        return;

    Pipe* pipe = pipes.front();
    Window* window = new Window( pipe );
    const PixelViewport& pvp = pipe->getPixelViewport( );
    _choosePixelViewport( *window, pvp );
    window->setName( pipe->getName() + " window" );
    window->setIAttribute( Window::IATTR_PLANES_STENCIL, 1 );

    Channel* channel = new Channel( window );
    channel->setName( pipe->getName() + " channel" );
    Observer* observer = new Observer( config );

    Wall wall;
    const PixelViewport& windowPvp = window->getPixelViewport( );
    if( windowPvp.isValid( ))
        wall.resizeHorizontalToAR( float( windowPvp.w ) / float( windowPvp.h ));
    else if( pvp.isValid( ))
        wall.resizeHorizontalToAR( float( pvp.w ) / float( pvp.h ));

    Canvas* canvas = new Canvas( config );
    canvas->setWall( wall );

    Segment* segment = new Segment( canvas );
    segment->setChannel( channel );

    Strings names;
    const Nodes& nodes = config->getNodes();
    const bool scalability = nodes.size() > 1 || pipes.size() > 1;

    if( scalability )
        names.push_back( EQ_SERVER_CONFIG_LAYOUT_2D_DYNAMIC );

    names.push_back( EQ_SERVER_CONFIG_LAYOUT_SIMPLE );

    if( scalability )
    {
        names.push_back( EQ_SERVER_CONFIG_LAYOUT_DB_DS );
        names.push_back( EQ_SERVER_CONFIG_LAYOUT_DB_STATIC );
        names.push_back( EQ_SERVER_CONFIG_LAYOUT_DB_DYNAMIC );
        names.push_back( EQ_SERVER_CONFIG_LAYOUT_2D_STATIC );
        names.push_back( EQ_SERVER_CONFIG_LAYOUT_SUBPIXEL );

        if( flags & fabric::ConfigParams::FLAG_MULTIPROCESS_DB &&
            nodes.size() > 1 )
        {
            for( NodesCIter i = nodes.begin(); i != nodes.end(); ++i )
            {
                if( (*i)->getPipes().size() > 1 )
                {
                    names.push_back( EQ_SERVER_CONFIG_LAYOUT_DB_2D );
                    break;
                }
            }
        }
    }

    for( StringsCIter i = names.begin(); i != names.end(); ++i )
    {
        Layout* layout = new Layout( config );
        layout->setName( *i );

        View* view = new View( layout );
        view->setObserver( observer );
        view->setWall( wall );

        canvas->addLayout( layout );
    }

    config->activateCanvas( canvas );
}

}
}
}
