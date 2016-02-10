
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

#include "dfrEqualizer.h"

#include "../compound.h"
#include "../compoundVisitor.h"
#include "../config.h"
#include "../log.h"

#include <eq/fabric/statistic.h>
#include <eq/fabric/zoom.h>
#include <lunchbox/debug.h>

namespace eq
{
namespace server
{
static const float MINSIZE = 128.f; // pixels

DFREqualizer::DFREqualizer()
        : _current ( getFrameRate( ))
        , _lastTime( 0 )
{
    LBINFO << "New DFREqualizer @" << (void*)this << std::endl;
}

DFREqualizer::~DFREqualizer()
{
    attach( 0 );
    LBINFO << "Delete DFREqualizer @" << (void*)this << std::endl;
}

void DFREqualizer::attach( Compound* compound )
{
    Compound* oldCompound = getCompound();
    if( oldCompound )
    {
        Channel*  channel   = oldCompound->getChannel();
        LBASSERT( channel );

        // Unsubscribe to channel load notification
        channel->removeListener( this );
    }

    Equalizer::attach( compound );

    if( compound )
    {
        Channel* channel = compound->getChannel();
        LBASSERT( channel );

        // Subscribe to channel load notification
        if( compound->getParent() && channel )
            channel->addListener( this );
    }
}

void DFREqualizer::notifyUpdatePre( Compound* compound, const uint32_t/*frame*/)
{
    LBASSERT( compound == getCompound( ));

    if( isFrozen() || !compound->isActive() || !isActive( ))
    {
        compound->setZoom( Zoom::NONE );
        return;
    }

    LBASSERT( getDamping() >= 0.f );
    LBASSERT( getDamping() <= 1.f );

    const float factor = ( sqrtf( _current / getFrameRate( )) - 1.f ) *
                         getDamping() + 1.f;

    Zoom newZoom( compound->getZoom( ));
    newZoom *= factor;

    //LBINFO << _current << ": " << factor << " = " << newZoom << std::endl;

    // clip zoom factor to min, max( channel pvp )
    const Compound*      parent = compound->getParent();
    const PixelViewport& pvp    = parent->getInheritPixelViewport();

    const Channel*       channel    = compound->getChannel();
    const PixelViewport& channelPVP = channel->getPixelViewport();

    const float minZoom = MINSIZE / LB_MIN( static_cast< float >( pvp.h ),
                                            static_cast< float >( pvp.w ));
    const float maxZoom = LB_MIN( static_cast< float >( channelPVP.w ) /
                                  static_cast< float >( pvp.w ),
                                  static_cast< float >( channelPVP.h ) /
                                  static_cast< float >( pvp.h ));

    newZoom.x() = LB_MAX( newZoom.x(), minZoom );
    newZoom.x() = LB_MIN( newZoom.x(), maxZoom );
    newZoom.y() = newZoom.x();

    compound->setZoom( newZoom );
}

void DFREqualizer::notifyLoadData( Channel* channel, const uint32_t frameNumber,
                                   const Statistics& statistics,
                                   const Viewport& /*region*/ )
{
    // gather and notify load data
    int64_t endTime = 0;
    for( size_t i = 0; i < statistics.size(); ++i )
    {
        const Statistic& data = statistics[i];
        switch( data.type )
        {
            case Statistic::CHANNEL_CLEAR:
            case Statistic::CHANNEL_DRAW:
            case Statistic::CHANNEL_ASSEMBLE:
            case Statistic::CHANNEL_READBACK:
                endTime = LB_MAX( endTime, data.endTime );
                break;

            default:
                break;
        }
    }

    if( endTime == 0 )
        return;

    const int64_t time = endTime - _lastTime;
    _lastTime = endTime;

    if( _lastTime <= 0 || time <= 0 )
        return;

    _current = 1000.0f / static_cast< float >( time );
    LBLOG( LOG_LB1 ) << "Frame " << frameNumber << " channel "
                     << channel->getName() << " time " << time << std::endl;
}

std::ostream& operator << ( std::ostream& os, const DFREqualizer* lb )
{
    if( !lb )
        return os;

    os << lunchbox::disableFlush
       << "DFR_equalizer " << std::endl
       << '{' << std::endl
       << "    framerate " << lb->getFrameRate() << std::endl;

    if( lb->getDamping() != 0.5f )
        os << "    damping " << lb->getDamping() << std::endl;

    os << '}' << std::endl << lunchbox::enableFlush;
    return os;
}

}
}
