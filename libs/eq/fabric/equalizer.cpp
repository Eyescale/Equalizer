
/* Copyright (c) 2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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
        : _damping( .5f )
        , _boundaryf( std::numeric_limits<float>::epsilon() )
        , _resistancef( .0f )
        , _assembleOnlyLimit( std::numeric_limits< float >::max( ) )
        , _target( 10.f )
        , _boundary2i( 1, 1 )
        , _resistance2i( 0, 0 )
        , _tilesize( 64, 64 )
        , _mode( fabric::Equalizer::MODE_2D )
        , _frozen( false )
    {}

    Equalizer( const Equalizer& rhs )
        : _damping( rhs._damping )
        , _boundaryf( rhs._boundaryf )
        , _resistancef( rhs._resistancef )
        , _assembleOnlyLimit( rhs._assembleOnlyLimit )
        , _target( rhs._target )
        , _boundary2i( rhs._boundary2i )
        , _resistance2i( rhs._resistance2i )
        , _tilesize( rhs._tilesize )
        , _mode( rhs._mode )
        , _frozen( rhs._frozen )
    {}

    float _damping;
    float _boundaryf;
    float _resistancef;
    float _assembleOnlyLimit;
    float _target;
    Vector2i _boundary2i;
    Vector2i _resistance2i;
    Vector2i _tilesize;
    fabric::Equalizer::Mode _mode;
    bool _frozen;
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

    _data->_damping = rhs._data->_damping;
    _data->_boundaryf = rhs._data->_boundaryf;
    _data->_resistancef = rhs._data->_resistancef;
    _data->_assembleOnlyLimit = rhs._data->_assembleOnlyLimit;
    _data->_target = rhs._data->_target;
    _data->_boundary2i = rhs._data->_boundary2i;
    _data->_resistance2i = rhs._data->_resistance2i;
    _data->_tilesize = rhs._data->_tilesize;
    _data->_mode = rhs._data->_mode;
    _data->_frozen = rhs._data->_frozen;

    return *this;
}

Equalizer::~Equalizer()
{
    delete _data;
}

void Equalizer::setFrozen( const bool onOff )
{
    _data->_frozen = onOff;
}

bool Equalizer::isFrozen() const
{
    return _data->_frozen;
}

void Equalizer::setMode( const Mode mode )
{
    _data->_mode = mode;
}

Equalizer::Mode Equalizer::getMode() const
{
    return _data->_mode;
}

void Equalizer::setDamping( const float damping )
{
    _data->_damping = damping;
}

float Equalizer::getDamping() const
{
    return _data->_damping;
}

void Equalizer::setFrameRate( const float frameRate )
{
    _data->_target = frameRate;
}

float Equalizer::getFrameRate() const
{
    return _data->_target;
}

void Equalizer::setBoundary( const Vector2i& boundary )
{
    LBASSERT( boundary.x() > 0 && boundary.y() > 0 );
    _data->_boundary2i = boundary;
}

void Equalizer::setBoundary( const float boundary )
{
    LBASSERT( boundary > 0.0f );
    _data->_boundaryf = boundary;
}

const Vector2i& Equalizer::getBoundary2i() const
{
    return _data->_boundary2i;
}

float Equalizer::getBoundaryf() const
{
    return _data->_boundaryf;
}

void Equalizer::setResistance( const Vector2i& resistance )
{
    _data->_resistance2i = resistance;
}

void Equalizer::setResistance( const float resistance )
{
    _data->_resistancef = resistance;
}

const Vector2i& Equalizer::getResistance2i() const
{
    return _data->_resistance2i;
}

float Equalizer::getResistancef() const
{
    return _data->_resistancef;
}

void Equalizer::setAssembleOnlyLimit( const float limit )
{
    _data->_assembleOnlyLimit = limit;
}

float Equalizer::getAssembleOnlyLimit() const
{
    return _data->_assembleOnlyLimit;
}

void Equalizer::setTileSize( const Vector2i& size )
{
    _data->_tilesize = size;
}

const Vector2i& Equalizer::getTileSize() const
{
    return _data->_tilesize;
}

void Equalizer::serialize( co::DataOStream& os )
{
    os << _data->_damping << _data->_boundaryf << _data->_resistancef
       << _data->_assembleOnlyLimit << _data->_target << _data->_boundary2i
       << _data->_resistance2i << _data->_tilesize << _data->_mode
       << _data->_frozen;
}

void Equalizer::deserialize( co::DataIStream& is )
{
    is >>_data->_damping >>_data->_boundaryf >>_data->_resistancef
       >>_data->_assembleOnlyLimit >>_data->_target >>_data->_boundary2i
       >>_data->_resistance2i >>_data->_tilesize >> _data->_mode
       >>_data->_frozen;
}

void Equalizer::backup()
{
    _backup = new detail::Equalizer( *_data );
}

void Equalizer::restore()
{
    delete _data;
    _data = _backup;
    _backup = 0;
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
