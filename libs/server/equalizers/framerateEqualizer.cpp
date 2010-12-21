
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "framerateEqualizer.h"

#include "../compound.h"
#include "../compoundVisitor.h"
#include "../config.h"
#include "../log.h"

#include <eq/statistic.h>
#include <co/base/debug.h>

#define USE_AVERAGE
#define VSYNC_CAP 60.f
#define SLOWDOWN  1.05f

namespace eq
{
namespace server
{

namespace
{
class LoadSubscriber : public CompoundVisitor
{
public:
    LoadSubscriber( ChannelListener* listener ) : _listener( listener ) {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel*  channel = compound->getChannel();
            EQASSERT( channel );
            channel->addListener( _listener );

            return TRAVERSE_CONTINUE; 
        }

private:
    ChannelListener* const _listener;
};

class LoadUnsubscriber : public CompoundVisitor
{
public:
    LoadUnsubscriber( ChannelListener* listener ) : _listener( listener ) {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel*  channel = compound->getChannel();
            EQASSERT( channel );
            channel->removeListener( _listener );

            return TRAVERSE_CONTINUE; 
        }

private:
    ChannelListener* const _listener;
};

}

// The smooth load balancer adapts the framerate of the compound to be the
// average frame rate of all children, taking the DPlex period into account.

FramerateEqualizer::FramerateEqualizer()
        : _nSamples( 0 )
{
    EQINFO << "New FramerateEqualizer @" << (void*)this << std::endl;
}

FramerateEqualizer::FramerateEqualizer( const FramerateEqualizer& from )
        : Equalizer( from )
        , _nSamples( 0 )
{
}

FramerateEqualizer::~FramerateEqualizer()
{
    attach( 0 );
}

void FramerateEqualizer::attach( Compound* compound )
{
    _exit();
    Equalizer::attach( compound );
}


void FramerateEqualizer::_init()
{
    const Compound* compound = getCompound();

    if( _nSamples > 0 || !compound )
        return;

    _nSamples = 1;

    // Subscribe to child channel load events
    const Compounds& children = compound->getChildren();
    
    EQASSERT( _loadListeners.empty( ));
    _loadListeners.resize( children.size( ));
    
    for( size_t i = 0; i < children.size(); ++i )
    {
        Compound*      child        = children[i];
        const uint32_t period       = child->getInheritPeriod();
        LoadListener&  loadListener = _loadListeners[i];
        
        loadListener.parent = this;
        loadListener.period = period;
        
        LoadSubscriber subscriber( &loadListener );
        child->accept( subscriber );
        
        _nSamples = EQ_MAX( _nSamples, period );
    }

    _nSamples = EQ_MIN( _nSamples, 100 );
}

void FramerateEqualizer::_exit()
{
    const Compound* compound = getCompound();
    if( !compound || _nSamples == 0 )
        return;

    const Compounds& children = compound->getChildren();

    EQASSERT( _loadListeners.size() == children.size( ));
    for( size_t i = 0; i < children.size(); ++i )
    {
        Compound*      child        = children[i];
        LoadListener&  loadListener = _loadListeners[i];
        
        LoadUnsubscriber unsubscriber( &loadListener );
        child->accept( unsubscriber );
    }
    
    _loadListeners.clear();
    _times.clear();
    _nSamples = 0;
}


void FramerateEqualizer::notifyUpdatePre( Compound* compound, 
                                          const uint32_t frameNumber )
{
    _init();

    // find starting point of contiguous block
    const ssize_t size = static_cast< ssize_t >( _times.size( ));
    ssize_t       from = 0;
    if( size > 0 )
    {
        for( ssize_t i = size-1; i >= 0; --i )
        {
            if( _times[i].second != 0.f )
                continue;

            from = i;
            break;
        }
    }

    // find max / avg time in block
    size_t nSamples = 0;
#ifdef USE_AVERAGE
    float sumTime = 0.f;
#else
    float maxTime  = 0.f;
#endif

    for( ++from; from < size && nSamples < _nSamples; ++from )
    {
        const FrameTime& time = _times[from];
        EQASSERT( time.first > 0 );
        EQASSERT( time.second != 0.f );

        ++nSamples;
#ifdef USE_AVERAGE
        sumTime += time.second;
#else
        maxTime = EQ_MAX( maxTime, time.second );
#endif
        EQLOG( LOG_LB2 ) << "Using " << time.first << ", " << time.second
                         << "ms" << std::endl;
    }

    if( nSamples == _nSamples )       // If we have a full set
        while( from < static_cast< ssize_t >( _times.size( )))
            _times.pop_back();            //  delete all older samples

    if( isFrozen() || !compound->isRunning( ))
    {
        // always execute code above to not leak memory
        compound->setMaxFPS( std::numeric_limits< float >::max( ));
        return;
    }

    if( nSamples > 0 )
    {
        //TODO: totalTime *= 1.f - damping;
#ifdef USE_AVERAGE
        const float time = (sumTime / nSamples) * SLOWDOWN;
#else
        const float time = maxTime * SLOWDOWN;
#endif

        const float fps = 1000.f / time;
#ifdef VSYNC_CAP
        if( fps > VSYNC_CAP )
            compound->setMaxFPS( std::numeric_limits< float >::max( ));
        else
#endif
            compound->setMaxFPS( fps );

        EQLOG( LOG_LB2 ) << fps << " Hz from " << nSamples << "/" 
                        << _times.size() << " samples, " << time << "ms" 
                        << std::endl;
    }

    _times.push_front( FrameTime( frameNumber, 0.f ));
    EQASSERT( _times.size() < 210 );
}

void FramerateEqualizer::LoadListener::notifyLoadData( 
                            Channel* channel,
                            const uint32_t frameNumber, 
                            const uint32_t nStatistics,
                            const eq::Statistic* statistics  )
{
    // gather required load data
    int64_t startTime = std::numeric_limits< int64_t >::max();
    int64_t endTime   = 0;
    for( uint32_t i = 0; i < nStatistics; ++i )
    {
        const eq::Statistic& data = statistics[i];
        switch( data.type )
        {
            case eq::Statistic::CHANNEL_CLEAR:
            case eq::Statistic::CHANNEL_DRAW:
            case eq::Statistic::CHANNEL_ASSEMBLE:
            case eq::Statistic::CHANNEL_READBACK:
                startTime = EQ_MIN( startTime, data.startTime );
                endTime   = EQ_MAX( endTime, data.endTime );
                break;
                
            default:
                break;
        }
    }
    
    if( startTime == std::numeric_limits< int64_t >::max( ))
        return;
    
    if( startTime == endTime ) // very fast draws might report 0 times
        ++endTime;

    for( std::deque< FrameTime >::iterator i = parent->_times.begin();
         i != parent->_times.end(); ++i )
    {
        FrameTime& frameTime = *i;
        if( frameTime.first != frameNumber )
            continue;

        const float time = static_cast< float >( endTime - startTime ) / period;
        frameTime.second = EQ_MAX( frameTime.second, time );
        EQLOG( LOG_LB2 ) << "Frame " << frameNumber << " channel " 
                        << channel->getName() << " time " << time
                        << " period " << period << std::endl;
    }
}

std::ostream& operator << ( std::ostream& os, const FramerateEqualizer* lb )
{
    if( lb )
        os << "framerate_equalizer {}" << std::endl;
    return os;
}

}
}
