
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

#define NB_ELEMENT 30
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
   
   Zoom currentZoom = _compound->getZoom();
   _average = (( _average * (NB_ELEMENT - 1) ) + _fpsLastFrame ) / NB_ELEMENT ;
   
   float factor = sqrtf( _average / _parent.getFrameRate() );

   // TODO: min/max on new zoom!
   // TODO: max zoom factor = 
   //          min x,y( my channel pvp / parent compound inherit pvp )
   // TODO: min zoom factor = never smaller than 128 pixels in both directions

   factor = EQ_MAX( factor, 0.5f );
   factor = EQ_MIN( factor, 2.0f ); 
   
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
