
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
        : _channel( 0 )
{
}

ViewEqualizer::Listener::Listener( const Listener& from )
        : ChannelListener()
        , _channel( from._channel )
{
    if( _channel )
        _channel->addListener( this );
}

const ViewEqualizer::Listener::Listener& 
ViewEqualizer::Listener::operator = ( const Listener& from )
{
    if( _channel )
        _channel->removeListener( this );

    _channel = from._channel;

    if( _channel )
        _channel->addListener( this );

    return *this;
}

ViewEqualizer::Listener::~Listener()
{
    if( _channel )
        _channel->removeListener( this );
}

void ViewEqualizer::attach( Compound* compound )
{
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
#if 0
    const Compound* compound = getCompound();
    const CompoundVector& children = compound->getChildren();
    const size_t nChildren = children.size();

    _listeners.resize( nChildren );
    for( size_t i = 0; i < nChildren; ++i )
    {
        Listener& listener = _listeners[ i ];
        
        listener->update( compound );
    }
#endif
}

std::ostream& operator << ( std::ostream& os, const ViewEqualizer* equalizer)
{
    if( equalizer )
        os << "view_equalizer {}" << std::endl;
    return os;
}

}
}
