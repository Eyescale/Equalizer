
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include "dfrLoadBalancer.h"

#include "compound.h"
#include "compoundVisitor.h"
#include "config.h"
#include "log.h"

#include <eq/base/debug.h>
#include <eq/client/zoom.h>
using namespace eq::base;
using namespace std;

#define QUICK_ADAPT
#define NB_ELEMENT_MAX 100

namespace eq
{
namespace server
{

DFRLoadBalancer::DFRLoadBalancer( const LoadBalancer& parent )
        : LoadBalancerIF( parent )
        , _compound( _parent.getCompound() )
        , _fpsLastFrame ( _parent.getFrameRate() )
        , _average ( _parent.getFrameRate() )
        , _newValueReady ( false )
        , _count ( 0 )
{    
    Channel* channel = _compound->getChannel();
    
    const float damping = EQ_MAX( _parent.getDamping(), 0.f );
    
    _sizeAverage = (int) ( NB_ELEMENT_MAX * damping ) + 1;
    
    EQASSERT( channel );
    // Subscribe to channel load notification
    if ( _compound->getParent() && channel)
        channel->addListener( this );

    EQINFO << "New DFRLoadBalancer @" << (void*)this << endl;
}

DFRLoadBalancer::~DFRLoadBalancer()
{

    Channel*  channel   = _compound->getChannel();

    EQASSERT( channel );
    // Unsubscribe to channel load notification
    channel->removeListener( this );
    EQINFO << "Remove DFRLoadBalancer @" << (void*)this << endl;
}

void DFRLoadBalancer::update( const uint32_t frameNumber )
{
    if ( _parent.isFrozen())
    {
        _compound->setZoom( Zoom::NONE );  
        return;    
    }
   
   Zoom newZoom = _compound->getZoom();

#ifdef QUICK_ADAPT
    if ( !_newValueReady )
        return;
   
   _newValueReady = false;    
   
   EQASSERT( _parent.getDamping() >= 0.f );
   EQASSERT( _parent.getDamping() <= 1.f );

   const float target = _parent.getFrameRate();
   const float factor = ( sqrtf( _fpsLastFrame / target ) - 1.f ) * 
                            _parent.getDamping() + 1.0f;

#else

   if ( _count <= _sizeAverage )
        return;

   _average = _average / (_count-1);
   _average = 0.f;
   _count   = 0;

   const float factor = sqrtf( _average / _parent.getFrameRate() );
   // EQINFO << "Frame " << frameNumber << " fps " << _average
   //                    << endl;
      
#endif

   newZoom *= factor;

   //EQINFO << _fpsLastFrame << ": " << factor << " = " << newZoom 
   //       << std::endl;

   // clip zoom factor to min( 128px ), max( channel pvp )

   const Compound*          parent = _compound->getParent();
   const eq::PixelViewport& pvp    = parent->getInheritPixelViewport();
   
   const Channel*           channel    = _compound->getChannel();
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
   
   _compound->setZoom( newZoom );
}

void DFRLoadBalancer::notifyLoadData( Channel* channel,
                                      const uint32_t frameNumber, 
                                      const uint32_t nStatistics,
                                      const eq::Statistic* statistics  )
{
    // gather and notify load data
    float startTime = numeric_limits< float >::max();
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
    
    if( startTime == numeric_limits< float >::max( ))
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

   EQLOG( LOG_LB ) << "Frame " << frameNumber << " channel " 
                        << channel->getName() << " time " << time
                        << endl;
}

}
}
