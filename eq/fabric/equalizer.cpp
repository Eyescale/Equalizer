
/* Copyright (c) 2012, Daniel Nachbaur <danielnachbaur@gmail.com>
 *               2012-2013, Stefan.Eilemann@epfl.ch
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

#include "equalizer.h"

#include "configParams.h"
#include "global.h"

#include <co/dataOStream.h>
#include <co/dataIStream.h>


namespace eq
{
namespace fabric
{
namespace detail
{
class Equalizer
{
public:
    Equalizer()
        : damping( .5f )
        , boundaryf( std::numeric_limits<float>::epsilon( ))
        , resistancef( .0f )
        , assembleOnlyLimit( std::numeric_limits< float >::max( ))
        , frameRate( 10.f )
        , boundary2i( 1, 1 )
        , resistance2i( 0, 0 )
        , tilesize( 64, 64 )
        , mode( fabric::Equalizer::MODE_2D )
        , frozen( false )
    {
        const uint32_t flags = eq::fabric::Global::getFlags();
        switch( flags & fabric::ConfigParams::FLAG_LOAD_EQ_ALL )
        {
            case fabric::ConfigParams::FLAG_LOAD_EQ_2D:
                mode = fabric::Equalizer::MODE_2D;
                break;
            case fabric::ConfigParams::FLAG_LOAD_EQ_HORIZONTAL:
                mode = fabric::Equalizer::MODE_HORIZONTAL;
                break;
            case fabric::ConfigParams::FLAG_LOAD_EQ_VERTICAL:
                mode = fabric::Equalizer::MODE_VERTICAL;
                break;
            case fabric::ConfigParams::FLAG_NONE:
                break;
            default:
                LBUNIMPLEMENTED;
        }
    }

    Equalizer( const Equalizer& rhs )
        : damping( rhs.damping )
        , boundaryf( rhs.boundaryf )
        , resistancef( rhs.resistancef )
        , assembleOnlyLimit( rhs.assembleOnlyLimit )
        , frameRate( rhs.frameRate )
        , boundary2i( rhs.boundary2i )
        , resistance2i( rhs.resistance2i )
        , tilesize( rhs.tilesize )
        , mode( rhs.mode )
        , frozen( rhs.frozen )
    {}

    float damping;
    float boundaryf;
    float resistancef;
    float assembleOnlyLimit;
    float frameRate;
    Vector2i boundary2i;
    Vector2i resistance2i;
    Vector2i tilesize;
    fabric::Equalizer::Mode mode;
    bool frozen;
};
}

Equalizer::Equalizer()
    : _data( new detail::Equalizer )
    , _backup( 0 )
{}

Equalizer::Equalizer( const Equalizer& rhs )
    : _data( new detail::Equalizer( *rhs._data ))
    , _backup( 0 )
{}

Equalizer& Equalizer::operator=( const Equalizer& rhs )
{
    if( this == &rhs )
        return *this;

    *_data = *rhs._data;
    return *this;
}

Equalizer::~Equalizer()
{
    delete _data;
}

void Equalizer::setFrozen( const bool onOff )
{
    _data->frozen = onOff;
}

bool Equalizer::isFrozen() const
{
    return _data->frozen;
}

void Equalizer::setMode( const Mode mode )
{
    _data->mode = mode;
}

Equalizer::Mode Equalizer::getMode() const
{
    return _data->mode;
}

void Equalizer::setDamping( const float damping )
{
    _data->damping = damping;
}

float Equalizer::getDamping() const
{
    return _data->damping;
}

void Equalizer::setFrameRate( const float frameRate )
{
    _data->frameRate = frameRate;
}

float Equalizer::getFrameRate() const
{
    return _data->frameRate;
}

void Equalizer::setBoundary( const Vector2i& boundary )
{
    LBASSERT( boundary.x() > 0 && boundary.y() > 0 );
    _data->boundary2i = boundary;
}

void Equalizer::setBoundary( const float boundary )
{
    LBASSERT( boundary > 0.0f );
    _data->boundaryf = boundary;
}

const Vector2i& Equalizer::getBoundary2i() const
{
    return _data->boundary2i;
}

float Equalizer::getBoundaryf() const
{
    return _data->boundaryf;
}

void Equalizer::setResistance( const Vector2i& resistance )
{
    _data->resistance2i = resistance;
}

void Equalizer::setResistance( const float resistance )
{
    _data->resistancef = resistance;
}

const Vector2i& Equalizer::getResistance2i() const
{
    return _data->resistance2i;
}

float Equalizer::getResistancef() const
{
    return _data->resistancef;
}

void Equalizer::setAssembleOnlyLimit( const float limit )
{
    _data->assembleOnlyLimit = limit;
}

float Equalizer::getAssembleOnlyLimit() const
{
    return _data->assembleOnlyLimit;
}

void Equalizer::setTileSize( const Vector2i& size )
{
    _data->tilesize = size;
}

const Vector2i& Equalizer::getTileSize() const
{
    return _data->tilesize;
}

void Equalizer::serialize( co::DataOStream& os ) const
{
    os << _data->damping << _data->boundaryf << _data->resistancef
       << _data->assembleOnlyLimit << _data->frameRate << _data->boundary2i
       << _data->resistance2i << _data->tilesize << _data->mode
       << _data->frozen;
}

void Equalizer::deserialize( co::DataIStream& is )
{
    is >> _data->damping >> _data->boundaryf >> _data->resistancef
       >> _data->assembleOnlyLimit >> _data->frameRate >> _data->boundary2i
       >> _data->resistance2i >> _data->tilesize >> _data->mode
       >> _data->frozen;
}

void Equalizer::backup()
{
    _backup = new detail::Equalizer( *_data );
}

void Equalizer::restore()
{
    LBASSERT( _backup );
    delete _data;
    _data = _backup;
    _backup = 0;
}

co::DataOStream& operator << ( co::DataOStream& os, const Equalizer& eq )
{
    eq.serialize( os );
    return os;
}

co::DataIStream& operator >> ( co::DataIStream& is, Equalizer& eq )
{
    eq.deserialize( is );
    return is;
}

std::ostream& operator << ( std::ostream& os, const Equalizer::Mode mode )
{
    os << ( mode == Equalizer::MODE_2D         ? "2D" :
            mode == Equalizer::MODE_VERTICAL   ? "VERTICAL" :
            mode == Equalizer::MODE_HORIZONTAL ? "HORIZONTAL" :
            mode == Equalizer::MODE_DB         ? "DB" : "ERROR" );
    return os;
}

}
}
