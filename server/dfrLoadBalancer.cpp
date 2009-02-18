
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "dfrLoadBalancer.h"

#include "compound.h"
#include "compoundVisitor.h"
#include "config.h"
#include "log.h"

#include <eq/base/debug.h>
#include <eq/client/zoom.h>
using namespace eq::base;
using namespace std;

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
{    
    Channel* channel = _compound->getChannel();
    
    float damping = EQ_MAX( _parent.getDamping(), 0.f );
    
    _sizeAverage = (int) ( NB_ELEMENT_MAX * damping ) + 1;
    
    EQASSERT( channel );
    // Subscribe to channel load notification
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
   
   if ( !_newValueReady )
       return;
   
   _newValueReady = false;    
   
   
   if ( _fpsLastFrame == _parent.getFrameRate())
   {
       _average = _fpsLastFrame;
       return;
   }
   
   Zoom currentZoom = _compound->getZoom();

   _average = (( _average * (_sizeAverage - 1) ) + _fpsLastFrame ) / _sizeAverage ;
   
   float factor = sqrtf( _average / _parent.getFrameRate() );

      
   // min/max on new zoom!
   // max zoom factor = 
   //          min x,y( my channel pvp / parent compound inherit pvp )
   // min zoom factor = never smaller than 128 pixels in both directions

   const eq::PixelViewport& pvp = _compound->getInheritPixelViewport();
   
   Channel*  channel   = _compound->getChannel();
   const eq::PixelViewport& channelPVP   = channel->getPixelViewport();
   
   float minZoom = 128.f / EQ_MIN( (float)pvp.h, (float)pvp.w );
   float maxZoom = EQ_MIN( (float)channelPVP.w / (float)pvp.w, (float)channelPVP.h / (float)pvp.h );
   
   factor = EQ_MAX( factor, minZoom ); 
   factor = EQ_MIN( factor, maxZoom );
   currentZoom *= factor;  

   
   _compound->setZoom( currentZoom );
   
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
             startTime = EQ_MIN( startTime, data.startTime );
             break;
            case eq::Statistic::CHANNEL_ASSEMBLE:
            case eq::Statistic::CHANNEL_READBACK:
#ifndef EQ_ASYNC_TRANSMIT
            case eq::Statistic::CHANNEL_TRANSMIT:
#endif
               
                endTime   = EQ_MAX( endTime, data.endTime );
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

    EQLOG( LOG_LB ) << "Frame " << frameNumber << " channel " 
                        << channel->getName() << " time " << time
                        << endl;
}

}
}
