
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

namespace eq
{
namespace server
{

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


void ViewEqualizer::Listener::notifyLoadData( Channel* channel, 
                                              const uint32_t frameNumber,
                                              const uint32_t nStatistics,
                                              const eq::Statistic* statistics )
{
    // TBD
}

std::ostream& operator << ( std::ostream& os, const ViewEqualizer* equalizer)
{
    if( equalizer )
        os << "view_equalizer {}" << std::endl;
    return os;
}

}
}
