
/* Copyright (c) 2009-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "../config.h"
#include "../log.h"
#include "../pipe.h"

#include <eq/statistic.h>

#include <set>

#define MIN_USAGE .1f // 10%

namespace eq
{
namespace server
{

ViewEqualizer::ViewEqualizer()
        : _nPipes( 0 )
{
    EQINFO << "New view equalizer @" << (void*)this << std::endl;
}

ViewEqualizer::ViewEqualizer( const ViewEqualizer& from )
        : Equalizer( from )
        , _nPipes( 0 )
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
    for( Listeners::iterator i =_listeners.begin(); i != _listeners.end(); ++i )
        (*i).clear();

    _listeners.clear();
    Equalizer::attach( compound );
}

void ViewEqualizer::notifyUpdatePre( Compound* compound, 
                                     const uint32_t frameNumber )
{
    EQASSERT( compound == getCompound( ));

    _updateListeners();
    _updateResources();
    _update( frameNumber );
}

namespace
{
class SelfAssigner : public CompoundVisitor
{
public:
    SelfAssigner( const Pipe* self, float& nResources, 
                  co::base::PtrHash< Pipe*, float >& pipeUsage )
            : _self( self ), _nResources( nResources ), _pipeUsage( pipeUsage )
            , _numChannels( 0 ) {}
    
    virtual VisitorResult visitLeaf( Compound* compound )
        {
            if( !compound->isRunning( ))
                return TRAVERSE_CONTINUE;

            Pipe* pipe = compound->getPipe();
            EQASSERT( pipe );

            if( pipe != _self )
                return TRAVERSE_CONTINUE;

            if( _pipeUsage.find( pipe ) == _pipeUsage.end( ))
                _pipeUsage[ pipe ] = 0.0f;
            
            float& pipeUsage = _pipeUsage[ pipe ];
            if( pipeUsage >= 1.0f )
            {
                compound->setUsage( 0.f );
                return TRAVERSE_TERMINATE;
            }

            if( pipeUsage > 0.0f ) // pipe already partly used
            {
                EQASSERT( pipeUsage < 1.0f );

                float use = 1.0f - pipeUsage;
                use = EQ_MAX( use, MIN_USAGE );

                compound->setUsage( use );
                _nResources -= use;
                pipeUsage = 1.0f; // Don't use more than twice
                EQLOG( LOG_LB1 ) << "  Use " 
                                << static_cast< unsigned >( use * 100.f + .5f )
                                << "% of " << pipe->getName() << " task "
                                << compound->getTaskID() << ", "
                                << _nResources * 100.f << "% left" << std::endl;
            }
            else
            {
                EQASSERT( pipeUsage == 0.0f );

                float use = EQ_MIN( 1.0f, _nResources );

                compound->setUsage( use );
                _nResources -= use;
                pipeUsage = use;
                EQLOG( LOG_LB1 ) << "  Use " 
                                << static_cast< unsigned >( use * 100.f + .5f )
                                << "% of " << pipe->getName() << " task "
                                << compound->getTaskID() << ", "
                                << _nResources * 100.f << "% left" << std::endl;
            }
            ++_numChannels;

            return TRAVERSE_TERMINATE;
        }
    
    uint32_t getNumChannels() const { return _numChannels; }

private:
    const Pipe* const _self;
    float& _nResources;
    co::base::PtrHash< Pipe*, float >& _pipeUsage;
    uint32_t _numChannels;
};

class PreviousAssigner : public CompoundVisitor
{
public:
    PreviousAssigner( const Pipe* self, float& nResources,
                      co::base::PtrHash< Pipe*, float >& pipeUsage )
            : _self( self ), _nResources( nResources ), _pipeUsage( pipeUsage )
            , _numChannels( 0 ) {}
    
    virtual VisitorResult visitLeaf( Compound* compound )
        {
            if( !compound->isRunning( ))
                return TRAVERSE_CONTINUE;

            Pipe* pipe = compound->getPipe();
            EQASSERT( pipe );
            
            if( compound->getUsage() == 0.0f || // not previously used
                pipe == _self)                  // already assigned above
            {
                return TRAVERSE_CONTINUE;
            }

            compound->setUsage( 0.0f ); // reset to unused
            if( _nResources <= MIN_USAGE ) // done
                return TRAVERSE_CONTINUE;

            if( _pipeUsage.find( pipe ) == _pipeUsage.end( ))
                _pipeUsage[ pipe ] = 0.0f;
            
            float& pipeUsage = _pipeUsage[ pipe ];
            if( pipeUsage > 0.0f ) // pipe already partly used
                return TRAVERSE_CONTINUE;

            float use = EQ_MIN( 1.0f, _nResources );
            if( use + MIN_USAGE > 1.0f )
                use = 1.0f;

            pipeUsage = use;
            compound->setUsage( use );
            _nResources -= use;
            ++_numChannels;

            EQLOG( LOG_LB1 ) << "  Use " 
                            << static_cast< unsigned >( use * 100.f + .5f )
                            << "% of " << pipe->getName() << " task "
                            << compound->getTaskID() << ", "
                            << _nResources * 100.f << "% left" << std::endl;
            return TRAVERSE_CONTINUE;
        }
    
    uint32_t getNumChannels() const { return _numChannels; }

private:
    const Pipe* const _self;
    float& _nResources;
    co::base::PtrHash< Pipe*, float >& _pipeUsage;
    uint32_t _numChannels;
};

class NewAssigner : public CompoundVisitor
{
public:
    NewAssigner( float& nResources, 
                 co::base::PtrHash< Pipe*, float >& pipeUsage )
            : _nResources( nResources ), _pipeUsage( pipeUsage )
            , _numChannels( 0 )
            , _fallback( 0 ) {}
    
    virtual VisitorResult visitLeaf( Compound* compound )
        {
            if( !compound->isRunning( ))
                return TRAVERSE_CONTINUE;

            if( !_fallback )
                _fallback = compound;

            if( compound->getUsage() != 0.0f ) // already used
                return TRAVERSE_CONTINUE;

            Pipe* pipe = compound->getPipe();
            EQASSERT( pipe );
            
            if( _pipeUsage.find( pipe ) == _pipeUsage.end( ))
                _pipeUsage[ pipe ] = 0.0f;
            
            float& pipeUsage = _pipeUsage[ pipe ];
            if( pipeUsage >= 1.0f )
                return TRAVERSE_CONTINUE;

            if( pipeUsage > 0.0f ) // pipe already partly used
            {
                EQASSERT( pipeUsage < 1.0f );

                float use = 1.0f - pipeUsage;
                use = EQ_MAX( use, MIN_USAGE );

                compound->setUsage( use );
                _nResources -= use;
                pipeUsage = 1.0f; // Don't use more than twice
                EQLOG( LOG_LB1 ) << "  Use " 
                                << static_cast< unsigned >( use * 100.f + .5f )
                                << "% of " << pipe->getName() << " task "
                                << compound->getTaskID() << ", "
                                << _nResources * 100.f << "% left" << std::endl;
            }
            else
            {
                EQASSERT( pipeUsage == 0.0f );

                float use = EQ_MIN( 1.0f, _nResources );

                compound->setUsage( use );
                _nResources -= use;
                pipeUsage = use;
                EQLOG( LOG_LB1 ) << "  Use " 
                                << static_cast< unsigned >( use * 100.f + .5f )
                                << "% of " << pipe->getName() << " task "
                                << compound->getTaskID() << ", "
                                << _nResources * 100.f << "% left" << std::endl;
            }
            ++_numChannels;

            if( _nResources <= MIN_USAGE )
                return TRAVERSE_TERMINATE; // done
            return TRAVERSE_CONTINUE;
        }
    
    uint32_t getNumChannels() const { return _numChannels; }
    Compound* getFallback() { return _fallback; }

private:
    float& _nResources;
    co::base::PtrHash< Pipe*, float >& _pipeUsage;
    uint32_t _numChannels;
    Compound* _fallback;
};

}

void ViewEqualizer::_update( const uint32_t frameNumber )
{
    const uint32_t frame = _findInputFrameNumber();
    EQLOG( LOG_LB1 ) << "Using data from frame " << frame << std::endl;

    //----- Gather data for frame
    Loads loads;
    int64_t totalTime( 0 );

    for( Listeners::iterator i =_listeners.begin(); i != _listeners.end(); ++i )
    {
        Listener& listener = *i;
        const Listener::Load& load = listener.useLoad( frame );

        totalTime += load.time;
        loads.push_back( load );
    }

    const Compound* compound = getCompound();

    if( isFrozen() || !compound->isRunning() || _nPipes == 0 )
        // always execute code above to not leak memory
        return;

    if( totalTime == 0 ) // no data
        totalTime = 1;

    const float resourceTime( static_cast< float >( totalTime ) / 
                              static_cast< float >( _nPipes ));
    EQLOG( LOG_LB1 ) << resourceTime << "ms/resource" << std::endl;

    //----- Assign new resource usage
    const Compounds& children = compound->getChildren();
    const size_t size( _listeners.size( ));
    EQASSERT( children.size() == size );
    co::base::PtrHash< Pipe*, float > pipeUsage;
    float* leftOvers = static_cast< float* >( alloca( size * sizeof( float )));

    // use self
    for( size_t i = 0; i < size; ++i )
    {
        Listener::Load& load = loads[ i ];
        EQASSERT( load.missing == 0 );

        Compound* child = children[ i ];
        if( !child->isRunning( ))
            continue;

        float segmentResources( load.time / resourceTime );

        EQLOG( LOG_LB1 ) << "----- balance step 1 for view " << i << " (" 
                        << child->getChannel()->getName() << ") using "
                        << segmentResources << " resources" << std::endl;
        SelfAssigner assigner( child->getPipe(), segmentResources, pipeUsage );
        
        child->accept( assigner );
        load.missing = assigner.getNumChannels();
        leftOvers[ i ] = segmentResources;
    }
    
    // use previous' frames resources
    for( size_t i = 0; i < size; ++i )
    {
        Listener::Load& load = loads[ i ];
        Compound* child = children[ i ];
        if( !child->isRunning( ))
            continue;

        float& leftOver = leftOvers[i];
        EQLOG( LOG_LB1 ) << "----- balance step 2 for view " << i << " (" 
                        << child->getChannel()->getName() << ") using "
                        << leftOver << " resources" << std::endl;
        PreviousAssigner assigner( child->getPipe(), leftOver, pipeUsage );
        
        child->accept( assigner );
        load.missing += assigner.getNumChannels();
    }
    
    // satisfy left-overs
    for( size_t i = 0; i < size; ++i )
    {
        Listener& listener = _listeners[ i ];
        EQASSERTINFO( listener.getNLoads() <= getConfig()->getLatency() + 3,
                      listener );

        float& leftOver = leftOvers[i];
        Listener::Load& load = loads[ i ];
        Compound* child = children[ i ];

        if( !child->isRunning( ))
            continue;

        if( leftOver > MIN_USAGE || load.missing == 0 )
        {
            EQLOG( LOG_LB1 ) << "----- balance step 3 for view " << i << " (" 
                            << child->getChannel()->getName() << ") using "
                            << leftOver << " resources" << std::endl;

            NewAssigner assigner( leftOver, pipeUsage );        
            child->accept( assigner );
            load.missing += assigner.getNumChannels();

            if( load.missing == 0 ) // assign at least one resource
            {
                Compound* fallback = assigner.getFallback();
                EQASSERT( fallback );
                EQASSERT( leftOver > 0 );

                fallback->setUsage( leftOver );
                load.missing = 1;
                EQLOG( LOG_LB1 ) << "  Use " 
                                << static_cast< unsigned >( leftOver*100.f+.5f )
                                << "% of " << fallback->getPipe()->getName()
                                << " task " << fallback->getTaskID()
                                << std::endl;
            }
        }

        listener.newLoad( frameNumber, load.missing );
    }
}

uint32_t ViewEqualizer::_findInputFrameNumber() const
{
    EQASSERT( !_listeners.empty( ));

    uint32_t frame = std::numeric_limits< uint32_t >::max();
    const Compound* compound = getCompound();
    const Compounds& children = compound->getChildren();
    const size_t nChildren = children.size();
    EQASSERT( nChildren == _listeners.size( ));

    for( size_t i = 0; i < nChildren; ++i )
    {
        const Compound* child = children[ i ];
        if( !child->isRunning( ))
            continue;

        const Listener& listener = _listeners[ i ];
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
    const Compounds& children = compound->getChildren();
    const size_t nChildren = children.size();

    _listeners.resize( nChildren );
    for( size_t i = 0; i < nChildren; ++i )
    {
        EQLOG( LOG_LB1 ) << co::base::disableFlush << "Tasks for view " << i
                         << ": ";
        Listener& listener = _listeners[ i ];        
        listener.update( children[i] );
        EQLOG(LOG_LB1) << std::endl << co::base::enableFlush;
    }
}

namespace
{
class PipeCounter : public CompoundVisitor
{
public:
    virtual VisitorResult visitPre( const Compound* compound )
        { return compound->isRunning() ? TRAVERSE_CONTINUE : TRAVERSE_PRUNE; }

    virtual VisitorResult visitLeaf( const Compound* compound )
        {
            if( !compound->isRunning( ))
                return TRAVERSE_PRUNE;

            const Pipe* pipe = compound->getPipe();
            EQASSERT( pipe );
            _pipes.insert( pipe );
            return TRAVERSE_CONTINUE; 
        }

    size_t getNPipes() const { return _pipes.size(); }

private:
    std::set< const Pipe* > _pipes;
};
}

void ViewEqualizer::_updateResources()
{
    if( _nPipes > 0 )
        return;

    PipeCounter counter;
    const Compound* compound = getCompound();
    compound->accept( counter );
    _nPipes = counter.getNPipes();
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
                    co::base::PtrHash< Channel*, uint32_t >& taskIDs ) 
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
                EQLOG( LOG_LB1 ) << _taskIDs[ channel ] << ' ';
            }
            else
            {
                EQASSERTINFO( 0, 
                              "View equalizer does not support using channel "<<
                              channel->getName() <<
                              " multiple times in one branch" );
            }
            return TRAVERSE_CONTINUE; 
        }

private:
    ChannelListener* const _listener;
    co::base::PtrHash< Channel*, uint32_t >& _taskIDs;
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

ViewEqualizer::Listener::Load ViewEqualizer::Listener::Load::NONE( 0, 0, 1 );
ViewEqualizer::Listener::Load::Load( const uint32_t frame_, 
                                     const uint32_t missing_,
                                     const int64_t time_ )
        : frame( frame_ ), missing( missing_ ), nResources( missing_ )
        , time( time_ ) {}

bool ViewEqualizer::Listener::Load::operator == ( const Load& rhs ) const
{
    return ( frame == rhs.frame && missing == rhs.missing && time == rhs.time );
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
    int64_t startTime = std::numeric_limits< int64_t >::max();
    int64_t endTime   = 0;
    bool  loadSet   = false;
    int64_t transmitTime = 0;
    for( uint32_t i = 0; i < nStatistics && !loadSet; ++i )
    {
        const eq::Statistic& data = statistics[i];
        if( data.task != taskID ) // data from another compound
            continue;
        
        switch( data.type )
        {
        case eq::Statistic::CHANNEL_CLEAR:
        case eq::Statistic::CHANNEL_DRAW:
        case eq::Statistic::CHANNEL_READBACK:
            startTime = EQ_MIN( startTime, data.startTime );
            endTime   = EQ_MAX( endTime, data.endTime );
            break;

        case Statistic::CHANNEL_FRAME_TRANSMIT:
            transmitTime += data.startTime - data.endTime;
            break;
        case Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN:
            transmitTime -= data.endTime - data.startTime;
            break;

            // assemble blocks on input frames, stop using subsequent data
        case eq::Statistic::CHANNEL_ASSEMBLE:
            loadSet = true;
            break;
                
        default:
            break;
        }
    }

    if( startTime == std::numeric_limits< int64_t >::max( ))
        return;
    
    EQASSERTINFO( load.missing > 0, load );

    const int64_t time = EQ_MAX(endTime - startTime, transmitTime );
    load.time += time;
    --load.missing;

    if( load.missing == 0 )
    {
        const float rTime = float( load.time ) / float( load.nResources );
        load.time = int64_t( rTime * sqrtf( float( load.nResources )));
    }

    EQLOG( LOG_LB1 ) << "Task " << taskID << ", added time " << time << " to "
                    << load << std::endl;
}

uint32_t ViewEqualizer::Listener::findYoungestLoad() const
{
    for( LoadDeque::const_iterator i = _loads.begin(); i != _loads.end(); ++i )
    {
        const Load& load = *i;
        if( load.missing == 0 )
            return load.frame;
    }
    return 0;
}

const ViewEqualizer::Listener::Load& 
ViewEqualizer::Listener::useLoad( const uint32_t frame )
{
    for( LoadDeque::iterator i = _loads.begin(); i != _loads.end(); ++i )
    {
        Load& load = *i;
        if( load.frame == frame )
        {
            EQASSERT( load.missing == 0 );
            if( load.time == 0 )
                load.time = 1;

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
                                       const uint32_t nChannels )
{
    EQASSERT( nChannels > 0 );
    _loads.push_front( Load( frameNumber, nChannels, 0 ));
}

std::ostream& operator << ( std::ostream& os, const ViewEqualizer* equalizer )
{
    if( equalizer )
        os << "view_equalizer {}" << std::endl;
    return os;
}

std::ostream& operator << ( std::ostream& os,
                            const ViewEqualizer::Listener& listener )
{
    os << co::base::disableFlush << "Listener" << std::endl
       << co::base::indent;
    for( ViewEqualizer::Listener::LoadDeque::const_iterator i = 
             listener._loads.begin(); i != listener._loads.end(); ++i )
    {
        os << *i << std::endl;
    }
    os << co::base::exdent << co::base::enableFlush;
    return os; 
}

std::ostream& operator << ( std::ostream& os, 
                            const ViewEqualizer::Listener::Load& load )
{
    os << "frame " << load.frame << " missing " << load.missing << " t " 
       << load.time;
    return os;
}

}
}
