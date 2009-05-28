
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

#include "viewEqualizer.h"

#include "../compound.h"
#include "../compoundVisitor.h"
#include "../log.h"

namespace eq
{
namespace server
{
namespace
{
typedef std::vector< ViewEqualizer::Listener::Load > LoadVector;
}

ViewEqualizer::ViewEqualizer()
{
    EQINFO << "New view equalizer @" << (void*)this << std::endl;
}

ViewEqualizer::ViewEqualizer( const ViewEqualizer& from )
        : Equalizer( from )
{}

ViewEqualizer::~ViewEqualizer()
{
    attach( 0 );
    EQINFO << "Delete view equalizer @" << (void*)this << std::endl;
}

ViewEqualizer::Listener::Listener()
{
}

ViewEqualizer::Listener::~Listener()
{
}

void ViewEqualizer::attach( Compound* compound )
{
    for( ListenerVector::iterator i = _listeners.begin();
         i != _listeners.end(); ++i )
    {
        (*i).clear();
    }
    _listeners.clear();

    Equalizer::attach( compound );
}

void ViewEqualizer::notifyUpdatePre( Compound* compound, 
                                     const uint32_t frameNumber )
{
    EQASSERT( compound == getCompound( ));

    _updateListeners();
    _update( frameNumber );
}

namespace
{
class PreviousAssigner : public CompoundVisitor
{
public:
    PreviousAssigner( float& nResources,
                      base::PtrHash< Channel*, float >& channelUsage )
            : _nResources( nResources ), _channelUsage( channelUsage )
            , _numChannels( 0 ) {}
    
    virtual VisitorResult visitLeaf( Compound* compound )
        {
            if( compound->getUsage() == 0.0f ) // not previously used
                return TRAVERSE_CONTINUE;

            if( _nResources == 0.0f ) // done
                compound->setUsage( 0.0f );

            Channel* channel = compound->getChannel();
            EQASSERT( channel );
            
            if( _channelUsage.find( channel ) == _channelUsage.end( ))
                _channelUsage[ channel ] = 0.0f;
            
            float& channelUsage = _channelUsage[ channel ];
            if( channelUsage > 0.0f ) // channel already partly used
                return TRAVERSE_CONTINUE;

            const float use = EQ_MIN( 1.0f, _nResources );
            EQLOG( LOG_LB ) << "Use " << static_cast< unsigned >( use * 100.f )
                            << "% of " << channel->getName() << std::endl;
            channelUsage = use;
            compound->setUsage( use );
            _nResources -= use;
            ++_numChannels;

            return TRAVERSE_CONTINUE;
        }
    
    uint32_t getNumChannels() const { return _numChannels; }

private:
    float& _nResources;
    base::PtrHash< Channel*, float >& _channelUsage;
    uint32_t _numChannels;
};

class NewAssigner : public CompoundVisitor
{
public:
    NewAssigner( float& nResources, 
                 base::PtrHash< Channel*, float >& channelUsage )
            : _nResources( nResources ), _channelUsage( channelUsage )
            , _numChannels( 0 ) {}
    
    virtual VisitorResult visitLeaf( Compound* compound )
        {
            if( compound->getUsage() != 0.0f ) // already used
                return TRAVERSE_CONTINUE;

            Channel* channel = compound->getChannel();
            EQASSERT( channel );
            
            if( _channelUsage.find( channel ) == _channelUsage.end( ))
                _channelUsage[ channel ] = 0.0f;
            
            float& channelUsage = _channelUsage[ channel ];

            if( channelUsage == 1.0f )
                return TRAVERSE_CONTINUE;

            if( channelUsage > 0.0f ) // channel already partly used
            {
                EQASSERT( channelUsage < 1.0f );

                const float use = 1.0f - channelUsage;
                compound->setUsage( use );
                _nResources -= use;
                channelUsage = 1.0f; // Don't use more than two times
                EQLOG( LOG_LB ) << "Use " 
                                << static_cast< unsigned >( use * 100.f )
                                << "% of " << channel->getName() << std::endl;
            }
            else
            {
                EQASSERT( channelUsage == 0.0f )

                const float use = EQ_MIN( 1.0f, _nResources );
                compound->setUsage( use );
                _nResources -= use;
                channelUsage = use;
                EQLOG( LOG_LB ) << "Use " 
                                << static_cast< unsigned >( use * 100.f )
                                << "% of " << channel->getName() << std::endl;
            }
            ++_numChannels;

            if( _nResources <= 0.f )
                return TRAVERSE_TERMINATE; // done
            return TRAVERSE_CONTINUE;
        }
    
    uint32_t getNumChannels() const { return _numChannels; }

private:
    float& _nResources;
    base::PtrHash< Channel*, float >& _channelUsage;
    uint32_t _numChannels;
};

}

void ViewEqualizer::_update( const uint32_t frameNumber )
{
    const uint32_t frame = _findInputFrameNumber();

    //----- Gather data for frame
    LoadVector loads;
    float totalTime( 0.0f );
    float nResources( 0.0f );

    for( ListenerVector::iterator i = _listeners.begin();
         i != _listeners.end(); ++i )
    {
        Listener& listener = *i;
        const Listener::Load& load = listener.useLoad( frame );

        totalTime += load.time;
        nResources += load.nResources;
        loads.push_back( load );
    }

    if( nResources == 0.0f ) // no data
    {
        EQASSERT( totalTime == 0.0f );
        totalTime = 1.0f;
        nResources = 1.0f;
    }

    const float resourceTime( totalTime / nResources );

    //----- Assign new resource usage
    const CompoundVector& children = getCompound()->getChildren();
    const size_t size( _listeners.size( ));
    EQASSERT( children.size() == size );
    base::PtrHash< Channel*, float > channelUsage;
    float* leftOvers = static_cast< float* >( alloca( size * sizeof( float )));

    // use previous' frames resources
    for( size_t i = 0; i < size; ++i )
    {
        EQLOG( LOG_LB ) << "----- balance step 1 for view " << i << std::endl;
        Listener::Load& load = loads[ i ];
        EQASSERT( load.missing == 0 );

        float segmentResources( load.time / resourceTime );
        PreviousAssigner assigner( segmentResources, channelUsage );
        Compound* child = children[ i ];
        
        child->accept( assigner );
        load.missing = assigner.getNumChannels();
        leftOvers[ i ] = segmentResources;
    }
    
    // satisfy left-overs
    for( size_t i = 0; i < size; ++i )
    {
        float& leftOver = leftOvers[i];
        if( leftOver <= 0.f )
            continue;

        EQLOG( LOG_LB ) << "----- balance step 2 for view " << i << std::endl;
        NewAssigner assigner( leftOver, channelUsage );
        Compound* child = children[ i ];
        
        child->accept( assigner );

        Listener::Load& load = loads[ i ];
        load.missing += assigner.getNumChannels();
        
        Listener& listener = _listeners[ i ];
        listener.newLoad( frameNumber, load.missing,
                          load.time / resourceTime - leftOver );
    }
}

uint32_t ViewEqualizer::_findInputFrameNumber() const
{
    EQASSERT( !_listeners.empty( ));

    uint32_t frame = std::numeric_limits< uint32_t >::max();

    for( ListenerVector::const_iterator i = _listeners.begin();
         i != _listeners.end(); ++i )
    {
        const Listener& listener = *i;
        const uint32_t youngest = listener.findYoungestLoad();
        frame = EQ_MIN( frame, youngest );
    }

    return frame;
}


void ViewEqualizer::_updateListeners()
{
    if( !_listeners.empty( ))
    {
        EQASSERT( getCompound()->getChildren().size() == _listeners.size( ));
        return;
    }

    Compound* compound = getCompound();
    const CompoundVector& children = compound->getChildren();
    const size_t nChildren = children.size();

    _listeners.resize( nChildren );
    for( size_t i = 0; i < nChildren; ++i )
    {
        Listener& listener = _listeners[ i ];        
        listener.update( compound );
    }
}

//---------------------------------------------------------------------------
// Per-child listener implementation
//---------------------------------------------------------------------------
namespace
{
class LoadSubscriber : public CompoundVisitor
{
public:
    LoadSubscriber( ChannelListener* listener, 
                    base::PtrHash< Channel*, uint32_t >& taskIDs ) 
            : _listener( listener )
            , _taskIDs( taskIDs ) {}

    virtual VisitorResult visitLeaf( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            EQASSERT( channel );

            if( _taskIDs.find( channel ) == _taskIDs.end( ))
            {
                channel->addListener( _listener );
                _taskIDs[ channel ] = compound->getTaskID();
            }
            else
                EQASSERTINFO( 0, 
                              "View equalizer does not support using channel "<<
                              channel->getName() <<
                              " multiple times in one branch" );

            return TRAVERSE_CONTINUE; 
        }

private:
    ChannelListener* const _listener;
    base::PtrHash< Channel*, uint32_t >& _taskIDs;
};

}

void ViewEqualizer::Listener::update( Compound* compound )
{
    EQASSERT( _taskIDs.empty( ));
    LoadSubscriber subscriber( this, _taskIDs );
    compound->accept( subscriber );
}

void ViewEqualizer::Listener::clear()
{
    for( TaskIDHash::const_iterator i = _taskIDs.begin(); 
         i != _taskIDs.end(); ++i )
    {
        i->first->removeListener( this );
    }
    _taskIDs.clear();
}

ViewEqualizer::Listener::Load ViewEqualizer::Listener::Load::NONE( 0, 0, 
                                                                   1.f, 1 );
ViewEqualizer::Listener::Load::Load( const uint32_t frame_, 
                                     const uint32_t missing_,
                                     const float time_, 
                                     const float nResources_ )
        : frame( frame_ ), missing( missing ), time( time_ )
        , nResources( nResources_ ) 
{}

bool ViewEqualizer::Listener::Load::operator == ( const Load& rhs ) const
{
    return ( frame == rhs.frame && missing == rhs.missing &&
             time == rhs.time && nResources == rhs.nResources );
}

void ViewEqualizer::Listener::notifyLoadData( Channel* channel, 
                                              const uint32_t frameNumber,
                                              const uint32_t nStatistics,
                                              const eq::Statistic* statistics )
{
    Load& load = _getLoad( frameNumber );
    if( load == Load::NONE )
        return;

    EQASSERT( _taskIDs.find( channel ) != _taskIDs.end( ));
    const uint32_t taskID = _taskIDs[ channel ];
    
    // gather relevant load data
    float startTime = std::numeric_limits< float >::max();
    float endTime   = 0.0f;
    bool  loadSet   = false;
    float transmitTime = 0.0f;
    
    for( uint32_t i = 0; i < nStatistics && !loadSet; ++i )
    {
        const eq::Statistic& data = statistics[i];
        if( data.task != taskID ) // data from another compound
            continue;
        
        switch( data.type )
        {
            case eq::Statistic::CHANNEL_TRANSMIT:
#ifdef EQ_ASYNC_TRANSMIT
                transmitTime = data.endTime - data.startTime;
                break;
#else
                // no break;
#endif
            case eq::Statistic::CHANNEL_CLEAR:
            case eq::Statistic::CHANNEL_DRAW:
            case eq::Statistic::CHANNEL_READBACK:
                startTime = EQ_MIN( startTime, data.startTime );
                endTime   = EQ_MAX( endTime, data.endTime );
                break;
                
            // assemble blocks on input frames, stop using subsequent data
            case eq::Statistic::CHANNEL_ASSEMBLE:
                loadSet = true;
                break;
                
            default:
                break;
        }
    }
    
    float time( 0.f );
    if( startTime != std::numeric_limits< float >::max( ))
    {
        time = endTime - startTime;
        time = EQ_MAX( time, transmitTime );
    }

    EQASSERT( load.missing > 0 );
    load.time += time;
    --load.missing;
}

uint32_t ViewEqualizer::Listener::findYoungestLoad() const
{
    for( LoadDeque::const_iterator i = _loads.begin(); i != _loads.end(); ++i )
    {
        const Load& load = *i;
        if( load.missing == 0 )
            return load.frame;
    }
    return std::numeric_limits< uint32_t >::max();
}

const ViewEqualizer::Listener::Load& 
ViewEqualizer::Listener::useLoad( const uint32_t frame )
{
    for( LoadDeque::iterator i = _loads.begin(); i != _loads.end(); ++i )
    {
        const Load& load = *i;
        if( load.frame == frame )
        {
            EQASSERT( load.missing == 0 );
            ++i;
            _loads.erase( i, _loads.end( ));
            return load;
        }
    }

    return Load::NONE;
}

ViewEqualizer::Listener::Load& 
ViewEqualizer::Listener::_getLoad( const uint32_t frame )
{
    for( LoadDeque::iterator i = _loads.begin(); i != _loads.end(); ++i )
    {
        const Load& load = *i;
        if( load.frame == frame )
            return *i;
    }

    return Load::NONE;
}

void ViewEqualizer::Listener::newLoad( const uint32_t frameNumber, 
                                       const uint32_t nChannels,
                                       const float nResources )
{
    _loads.push_front( Load( frameNumber, nChannels, 0.0f, nResources ));
}

std::ostream& operator << ( std::ostream& os, const ViewEqualizer* equalizer)
{
    if( equalizer )
        os << "view_equalizer {}" << std::endl;
    return os;
}

}
}
