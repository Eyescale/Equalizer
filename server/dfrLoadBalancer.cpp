
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

namespace eq
{
namespace server
{


DfrLoadBalancer::DfrLoadBalancer( const LoadBalancer& parent )
        : LoadBalancerIF( parent )

         
{
    _compound = _parent.getCompound();
    
    _fpsLastFrame = _parent.getFrameRate();
     
    Channel*  channel   = _compound->getChannel();

    EQASSERT( channel );
    // Subscribe to channel load notification
    channel->addListener( this );
    EQINFO << "New DFRLoadBalancer @" << (void*)this << endl;
}

DfrLoadBalancer::~DfrLoadBalancer()
{

    Channel*  channel   = _compound->getChannel();

    EQASSERT( channel );
    // Unsubscribe to channel load notification
    channel->removeListener( this );
    EQINFO << "Remove DFRLoadBalancer @" << (void*)this << endl;
}

void DfrLoadBalancer::update( const uint32_t frameNumber )
{
   Zoom& currentZoom = _compound->getZoom();
   
   float factor;
   
   factor = sqrtf( _fpsLastFrame / _parent.getFrameRate() );
   
   if ( abs ( factor - 1.0f ) > 0.1 )
       currentZoom.applyFactor( factor );
   
   _fpsLastFrame = _parent.getFrameRate();

}


void DfrLoadBalancer::notifyLoadData( 
                            Channel* channel,
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
         

    
    _fpsLastFrame = 1000.0f / time;

    EQLOG( LOG_LB ) << "Frame " << frameNumber << " channel " 
                        << channel->getName() << " time " << time
                        << endl;
}

}
}
