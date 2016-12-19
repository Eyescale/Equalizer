
/* Copyright (c) 2009-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "layout.h"

#include "canvas.h"
#include "channel.h"
#include "config.h"
#include "segment.h"
#include "view.h"
#include "window.h"

#include <eq/fabric/commands.h>
#include <eq/fabric/paths.h>
#include <co/dataOStream.h>

namespace eq
{
namespace server
{
typedef fabric::Layout< Config, Layout, View > Super;

Layout::Layout( Config* parent )
    : Super( parent )
    , _state( STATE_ACTIVE ){}

Layout::~Layout(){}

ServerPtr Layout::getServer()
{
    Config* config = getConfig();
    LBASSERT( config );
    return ( config ? config->getServer() : 0 );
}

void Layout::postDelete()
{
    _state = STATE_DELETE;
    getConfig()->postNeedsFinish();
}

void Layout::notifyViewportChanged()
{
    for( View* view : getViews( ))
    {
        const Viewport& vp = view->getViewport();
        for( Channel* channel : view->getChannels( ))
        {
            Window* window = channel->getWindow();
            PixelViewport windowPVP = window->getPixelViewport();
            PixelViewport channelPVP = getPixelViewport();

            Segment* segment = channel->getSegment();
            const auto segmentVP = segment ? segment->getViewport() :Viewport();
            const Viewport coverage = vp.getCoverage( segmentVP );

            channelPVP.apply( vp );
            channelPVP.apply( coverage );
            channelPVP.x = channel->getPixelViewport().x;
            channelPVP.y = channel->getPixelViewport().y;

            if( channel->hasFixedViewport( ))
            {
                // resize window to gain target channel pvp
                const auto& channelVP = channel->getViewport();
                windowPVP.w = channelPVP.w / channelVP.w;
                windowPVP.h = channelPVP.h / channelVP.h;
                window->send( fabric::CMD_WINDOW_RESIZE ) << windowPVP;
                window->setPixelViewport( windowPVP );
            }
            else
            {
                // resize channel to fixed size, grow window size if needed
                if( windowPVP.w < channelPVP.getXEnd() ||
                    windowPVP.h < channelPVP.getYEnd( ))
                {
                    windowPVP.w = std::max( windowPVP.w, channelPVP.getXEnd( ));
                    windowPVP.h = std::max( windowPVP.h, channelPVP.getYEnd( ));
                    window->send( fabric::CMD_WINDOW_RESIZE ) << windowPVP;
                    window->setPixelViewport( windowPVP );
                }
                channel->setPixelViewport( channelPVP );
            }
        }
    }
}

void Layout::trigger( const Canvas* canvas, const bool active )
{
    LBASSERT( canvas );
    getConfig()->postNeedsFinish();

    for( View* view : getViews( ))
        view->trigger( canvas, active );
}

}
}

#include "nodeFactory.h"
#include "../fabric/layout.ipp"

template class eq::fabric::Layout< eq::server::Config, eq::server::Layout,
                                   eq::server::View >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
    const eq::fabric::Layout< eq::server::Config, eq::server::Layout,
                              eq::server::View >& );
/** @endcond */
