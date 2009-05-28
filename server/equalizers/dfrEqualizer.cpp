
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/base/debug.h>
#include <eq/client/zoom.h>

#define QUICK_ADAPT
#define MAX_ELEMENTS 100

namespace eq
{
namespace server
{

DFREqualizer::DFREqualizer()
        : _frameRate( 10.f )
        , _damping( .5f )
        , _fpsLastFrame ( _frameRate )
        , _average ( _frameRate )
        , _sizeAverage( 0 )
        , _newValueReady ( false )
        , _count ( 0 )
{    
    EQINFO << "New DFREqualizer @" << (void*)this << std::endl;
}

DFREqualizer::~DFREqualizer()
{
    attach( 0 );
    EQINFO << "Delete DFREqualizer @" << (void*)this << std::endl;
}

void DFREqualizer::attach( Compound* compound )
{
    Compound* oldCompound = getCompound();
    if( oldCompound )
    {
        Channel*  channel   = oldCompound->getChannel();
        EQASSERT( channel );

        // Unsubscribe to channel load notification
        channel->removeListener( this );
    }

    Equalizer::attach( compound );
    
    if( compound )
    {
        _sizeAverage = (int) ( MAX_ELEMENTS * _damping ) + 1;

        Channel* channel = compound->getChannel();
        EQASSERT( channel );
    
        // Subscribe to channel load notification
        if ( compound->getParent() && channel )
            channel->addListener( this );
    }
}

void DFREqualizer::notifyUpdatePre( Compound* compound, 
                                    const uint32_t frameNumber )
{
    EQASSERT( compound == getCompound( ));

    if( isFrozen( ))
    {
        compound->setZoom( Zoom::NONE );  
        return;    
    }
   
#ifdef QUICK_ADAPT
    if ( !_newValueReady )
        return;
   
   _newValueReady = false;    
   
   EQASSERT( _damping >= 0.f );
   EQASSERT( _damping <= 1.f );

   const float factor = ( sqrtf( _fpsLastFrame / _frameRate ) - 1.f ) * 
                            _damping + 1.0f;

#else

   if ( _count <= _sizeAverage )
        return;

   _average = _average / (_count-1);
   _average = 0.f;
   _count   = 0;

   const float factor = sqrtf( _average / _parent.getFrameRate() );
   // EQINFO << "Frame " << frameNumber << " fps " << _average
   //                    << std::endl;
      
#endif

   Zoom newZoom( compound->getZoom( ));
   newZoom *= factor;

   //EQINFO << _fpsLastFrame << ": " << factor << " = " << newZoom 
   //       << std::endl;

   // clip zoom factor to min( 128px ), max( channel pvp )

   const Compound*          parent = compound->getParent();
   const eq::PixelViewport& pvp    = parent->getInheritPixelViewport();
   
   const Channel*           channel    = compound->getChannel();
   const eq::PixelViewport& channelPVP = channel->getPixelViewport();
   
   const float minZoom = 128.f / EQ_MIN( static_cast< float >( pvp.h ),
                                         static_cast< float >( pvp.w ));
   const float maxZoom = EQ_MIN( static_cast< float >( channelPVP.w ) /
                                 static_cast< float >( pvp.w ),
                                 static_cast< float >( channelPVP.h ) /
                                 static_cast< float >( pvp.h ));
   
   newZoom.x = EQ_MAX( newZoom.x, minZoom ); 
   newZoom.x = EQ_MIN( newZoom.x, maxZoom );
   newZoom.y = newZoom.x; 
   
   compound->setZoom( newZoom );
}

void DFREqualizer::notifyLoadData( Channel* channel, const uint32_t frameNumber,
                                   const uint32_t nStatistics,
                                   const eq::Statistic* statistics  )
{
    // gather and notify load data
    float startTime = std::numeric_limits< float >::max();
    float endTime   = 0.0f;
    for( uint32_t i = 0; i < nStatistics; ++i )
    {
        const eq::Statistic& data = statistics[i];
        switch( data.type )
        {
            case eq::Statistic::CHANNEL_CLEAR:
            case eq::Statistic::CHANNEL_DRAW:
            case eq::Statistic::CHANNEL_ASSEMBLE:
            case eq::Statistic::CHANNEL_READBACK:
#ifndef EQ_ASYNC_TRANSMIT
            case eq::Statistic::CHANNEL_TRANSMIT:
#endif
                startTime = EQ_MIN( startTime, data.startTime );               
                endTime   = EQ_MAX( endTime,   data.endTime );
                break;
                
            default:
                break;
        }
    }
    
    if( startTime == std::numeric_limits< float >::max( ))
        return;
    
    const float time = endTime - startTime;
    
    if ( time <= 0.0f ) 
        return;
         
    _newValueReady = true;
    _fpsLastFrame = 1000.0f / time;

#ifndef QUICK_ADAPT
    _average = _average + _fpsLastFrame;
    ++_count;
#endif

   EQLOG( LOG_LB1 ) << "Frame " << frameNumber << " channel "
                    << channel->getName() << " time " << time << std::endl;
}

std::ostream& operator << ( std::ostream& os, const DFREqualizer* lb )
{
    if( !lb )
        return os;

    os << base::disableFlush
       << "DFR_equalizer " << std::endl
       << '{' << std::endl
       << "    framerate " << lb->getFrameRate() << std::endl;

    if( lb->getDamping() != 0.5f )
        os << "    damping " << lb->getDamping() << std::endl;
    
    os << '}' << std::endl << base::enableFlush;
    return os;
}

}
}
