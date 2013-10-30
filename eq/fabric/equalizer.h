
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

#ifndef EQFABRIC_EQUALIZER_H
#define EQFABRIC_EQUALIZER_H

#include <eq/fabric/api.h>
#include <eq/fabric/vmmlib.h>

namespace co
{
class DataOStream;
class DataIStream;
}


namespace eq
{
namespace fabric
{
namespace detail { class Equalizer; }

/** Base data transport class for equalizers. @sa eq::server::Equalizer */
class Equalizer
{
public:
    /** @internal */
    EQFABRIC_API Equalizer();

    /** @internal */
    EQFABRIC_API Equalizer( const Equalizer& rhs );

    /** @internal */
    EQFABRIC_API Equalizer& operator=( const Equalizer& rhs );

    /** @internal */
    EQFABRIC_API virtual ~Equalizer();

    enum Mode
    {
        MODE_DB = 0,     //!< Adapt for a sort-last decomposition
        MODE_HORIZONTAL, //!< Adapt for sort-first using horizontal stripes
        MODE_VERTICAL,   //!< Adapt for sort-first using vertical stripes
        MODE_2D          //!< Adapt for a sort-first decomposition
    };

    /** @name Data Access. */
    //@{
    /** Set the equalizer to freeze the current state. */
    EQFABRIC_API void setFrozen( const bool onOff );

    /** @return the equalizer frozen state. */
    EQFABRIC_API bool isFrozen() const;

    /** Set the load balancer adaptation mode. */
    EQFABRIC_API void setMode( const Mode mode );

    /** @return the load balancer adaptation mode. */
    EQFABRIC_API Mode getMode() const;

    /** Set the damping factor for the viewport, range or zoom adjustment.*/
    EQFABRIC_API void setDamping( const float damping );

    /** @return the damping factor. */
    EQFABRIC_API float getDamping() const;

    /** Set the average frame rate for the DFREqualizer. */
    EQFABRIC_API void setFrameRate( const float frameRate );

    /** @return the average frame rate for the DFREqualizer. */
    EQFABRIC_API float getFrameRate() const;

    /** Set a boundary for 2D tiles. */
    EQFABRIC_API void setBoundary( const Vector2i& boundary );

    /** Set a boundary for DB ranges. */
    EQFABRIC_API void setBoundary( const float boundary );

    /** @return the boundary for 2D tiles. */
    EQFABRIC_API const Vector2i& getBoundary2i() const;

    /** @return the boundary for DB ranges. */
    EQFABRIC_API float getBoundaryf() const;

    /** Set a resistance for 2D tiles. */
    EQFABRIC_API void setResistance( const Vector2i& resistance );

    /** Set a resistance for DB ranges. */
    EQFABRIC_API void setResistance( const float resistance );

    /** @return the resistance for 2D tiles. */
    EQFABRIC_API const Vector2i& getResistance2i() const;

    /** @return the resistance for DB ranges. */
    EQFABRIC_API float getResistancef() const;

    /** Set the limit when to assign assemble tasks only. */
    EQFABRIC_API void setAssembleOnlyLimit( const float limit );

    /** @return the limit when to assign assemble tasks only. */
    EQFABRIC_API float getAssembleOnlyLimit() const;

    /** Set the tile size for the TileEqualizer. */
    EQFABRIC_API void setTileSize( const Vector2i& size );

    /** @return the tile size for the TileEqualizer. */
    EQFABRIC_API const Vector2i& getTileSize() const;
    //@}

    EQFABRIC_API void serialize( co::DataOStream& os ) const; //!< @internal
    EQFABRIC_API void deserialize( co::DataIStream& is ); //!< @internal

    EQFABRIC_API void backup(); //!< @internal
    EQFABRIC_API void restore(); //!< @internal

private:
    detail::Equalizer* _data;
    detail::Equalizer* _backup;
};

EQFABRIC_API co::DataOStream& operator << ( co::DataOStream& os,
                                            const Equalizer& );

EQFABRIC_API co::DataIStream& operator >> ( co::DataIStream& is,
                                            Equalizer& );

EQFABRIC_API std::ostream& operator << ( std::ostream& os,
                                         const Equalizer::Mode );
}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::Equalizer::Mode& value )
    { byteswap( reinterpret_cast< uint32_t& >( value )); }
}

#endif // EQFABRIC_EQUALIZER_H
