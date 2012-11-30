
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
        Equalizer();

        /** @internal */
        Equalizer( const Equalizer& rhs );

        /** @internal */
        Equalizer& operator=( const Equalizer& rhs );

        /** @internal */
        virtual ~Equalizer();

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
        void setFrozen( const bool onOff );

        /** @return the equalizer frozen state. */
        bool isFrozen() const;

        /** Set the load balancer adaptation mode. */
        void setMode( const Mode mode );

        /** @return the load balancer adaptation mode. */
        Mode getMode() const;

        /** Set the damping factor for the viewport, range or zoom adjustment.*/
        void setDamping( const float damping );

        /** @return the damping factor. */
        float getDamping() const;

        /** Set the average frame rate for the DFREqualizer. */
        void setFrameRate( const float frameRate );

        /** @return the average frame rate for the DFREqualizer. */
        float getFrameRate() const;

        /** Set a boundary for 2D tiles. */
        void setBoundary( const Vector2i& boundary );

        /** Set a boundary for DB ranges. */
        void setBoundary( const float boundary );

        /** @return the boundary for 2D tiles. */
        const Vector2i& getBoundary2i() const;

        /** @return the boundary for DB ranges. */
        float getBoundaryf() const;

        /** Set a resistance for 2D tiles. */
        void setResistance( const Vector2i& resistance );

        /** Set a resistance for DB ranges. */
        void setResistance( const float resistance );

        /** @return the resistance for 2D tiles. */
        const Vector2i& getResistance2i() const;

        /** @return the resistance for DB ranges. */
        float getResistancef() const;

        /** Set the limit when to assign assemble tasks only. */
        void setAssembleOnlyLimit( const float limit );

        /** @return the limit when to assign assemble tasks only. */
        float getAssembleOnlyLimit() const;

        /** Set the tile size for the TileEqualizer. */
        void setTileSize( const Vector2i& size );

        /** @return the tile size for the TileEqualizer. */
        const Vector2i& getTileSize() const;
        //@}

        void serialize( co::DataOStream& os ); //!< @internal
        void deserialize( co::DataIStream& is ); //!< @internal

    private:
        detail::Equalizer* const _impl;
    };

    std::ostream& operator << ( std::ostream& os, const Equalizer::Mode );
}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::Equalizer::Mode& value )
    { byteswap( reinterpret_cast< uint32_t& >( value )); }
}

#endif // EQFABRIC_EQUALIZER_H
