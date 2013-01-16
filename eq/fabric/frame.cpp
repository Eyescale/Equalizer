
/* Copyright (c) 2012, Stefan Eilemann <eile@eyescale.ch>
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

#include "frame.h"

#include "zoom.h"
#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace fabric
{
struct ToNodes
{
    std::vector< uint128_t > inputNodes;
    std::vector< uint128_t > inputNetNodes;
};

namespace detail
{
class Frame
{
public:
    // shared data:
    std::string name;
    Vector2i offset;
    Zoom zoom;
    co::ObjectVersion frameDataVersion[ NUM_EYES ];
    ToNodes toNodes[ NUM_EYES ];

    Frame() : offset( Vector2i::ZERO ) {}

    void serialize( co::DataOStream& os ) const
    {
        os << name << offset << zoom;

        for( unsigned i = 0; i < NUM_EYES; ++i )
            os << frameDataVersion[i] << toNodes[i].inputNodes 
               << toNodes[i].inputNetNodes;
    }

    void deserialize( co::DataIStream& is )
    {
        is >> name >> offset >> zoom;

        for( unsigned i = 0; i < NUM_EYES; ++i )
            is >> frameDataVersion[i] >> toNodes[i].inputNodes
               >> toNodes[i].inputNetNodes;
    }
};
}
Frame::Frame()
        : _impl( new detail::Frame )
{}

Frame::~Frame()
{
    delete _impl;
}

void Frame::getInstanceData( co::DataOStream& os )
{
    _impl->serialize( os );
}

void Frame::applyInstanceData( co::DataIStream& is )
{
    _impl->deserialize( is );
}

void Frame::setName( const std::string& name )
{
    _impl->name = name;
}

const std::string& Frame::getName() const
{
    return _impl->name;
}

const Vector2i& Frame::getOffset() const
{
    return _impl->offset;
}

void Frame::setOffset( const Vector2i& offset )
{
    _impl->offset = offset;
}

void Frame::setZoom( const Zoom& zoom )
{
    _impl->zoom = zoom;
}

const Zoom& Frame::getZoom() const
{
    return _impl->zoom;
}

void Frame::_setDataVersion( const unsigned i, const co::ObjectVersion& ov )
{
    _impl->frameDataVersion[ i ] = ov;
}

const co::ObjectVersion& Frame::getDataVersion( const Eye eye ) const
{
    return _impl->frameDataVersion[ lunchbox::getIndexOfLastBit( eye )];
}

const std::vector< uint128_t >& Frame::getInputNodes( const Eye eye ) const
{
    return _impl->toNodes[ lunchbox::getIndexOfLastBit( eye )].inputNodes;
}

const std::vector< uint128_t >& Frame::getInputNetNodes(const Eye eye) const
{
    return _impl->toNodes[ lunchbox::getIndexOfLastBit( eye )].inputNetNodes;
}

std::vector< uint128_t >& Frame::_getInputNodes( const unsigned i )
{
    return _impl->toNodes[ i ].inputNodes;
}

std::vector< uint128_t >& Frame::_getInputNetNodes( const unsigned i )
{
    return _impl->toNodes[ i ].inputNetNodes;
}

std::ostream& operator << ( std::ostream& os, const Frame& frame )
{
    os << lunchbox::disableFlush << "frame" << std::endl
       << "{" << std::endl << lunchbox::indent
       << "name     \"" << frame.getName() << "\"" << std::endl;

    const Zoom& zoom = frame.getZoom();
    if( zoom.isValid() && zoom != Zoom::NONE )
        os << zoom << std::endl;

    return os << lunchbox::exdent << "}" << std::endl << lunchbox::enableFlush;
}

std::ostream& operator << ( std::ostream& os, const Frame::Type type )
{
    os << "type     ";
    if ( type == Frame::TYPE_TEXTURE ) 
        os << " texture" << std::endl;
    else if ( type == Frame::TYPE_MEMORY ) 
        os << " memory" << std::endl;
        
    return os;
}

std::ostream& operator << ( std::ostream& os, const Frame::Buffer buffer )
{
    if( buffer == Frame::BUFFER_NONE )
        os << "none ";
    else if( buffer & Frame::BUFFER_UNDEFINED )
        os << "undefined ";
    else
    {
        if( buffer & Frame::BUFFER_COLOR )
            os << "color ";
        if( buffer & Frame::BUFFER_DEPTH )
            os << "depth ";
    }

    return os;
}

}
}
