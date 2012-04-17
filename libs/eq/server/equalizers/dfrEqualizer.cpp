
/* Copyright (c) 2009-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/client/statistic.h>
#include <eq/fabric/zoom.h>
#include <lunchbox/debug.h>

namespace eq
{
namespace server
{

DFREqualizer::DFREqualizer()
        : _target( 10.f )
        , _damping( .5f )
        , _current ( _target )
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

void DFREqualizer::notifyUpdatePre( Compound* compound, 
                                    const uint32_t frameNumber )
{
    LBASSERT( compound == getCompound( ));

    if( isFrozen() || !compound->isRunning() || !isActive( ))
    {
        compound->setZoom( Zoom::NONE );  
        return;    
    }
   
    LBASSERT( _damping >= 0.f );
    LBASSERT( _damping <= 1.f );

    const float factor = ( sqrtf( _current / _target ) - 1.f ) * 
        _damping + 1.0f;

    Zoom newZoom( compound->getZoom( ));
    newZoom *= factor;

    //LBINFO << _current << ": " << factor << " = " << newZoom 
    //       << std::endl;

    // clip zoom factor to min( 128px ), max( channel pvp )

    const Compound*          parent = compound->getParent();
    const eq::PixelViewport& pvp    = parent->getInheritPixelViewport();
   
    const Channel*           channel    = compound->getChannel();
    const eq::PixelViewport& channelPVP = channel->getPixelViewport();
   
    const float minZoom = 128.f / LB_MIN( static_cast< float >( pvp.h ),
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
                                   const uint32_t nStatistics,
                                   const eq::Statistic* statistics,
                                   const Viewport& region )
{
    // gather and notify load data
    int64_t endTime = 0;
    for( uint32_t i = 0; i < nStatistics; ++i )
    {
        const eq::Statistic& data = statistics[i];
        switch( data.type )
        {
            case eq::Statistic::CHANNEL_CLEAR:
            case eq::Statistic::CHANNEL_DRAW:
            case eq::Statistic::CHANNEL_ASSEMBLE:
            case eq::Statistic::CHANNEL_READBACK:
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
