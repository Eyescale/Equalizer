
/* Copyright (c) 2008-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Carsten Rohn <carsten.rohn@rtt.ag>
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

#ifndef EQS_TILEEQUALIZER_H
#define EQS_TILEEQUALIZER_H

#include "equalizers/equalizer.h"

namespace eq
{
namespace server
{

class TileEqualizer;
std::ostream& operator << ( std::ostream& os, const TileEqualizer* );

class TileEqualizer : public Equalizer
{
public:
    EQSERVER_API TileEqualizer();
    TileEqualizer( const TileEqualizer& from );
    ~TileEqualizer() {};

    /** @sa CompoundListener::notifyUpdatePre */
    virtual void notifyUpdatePre( Compound* compound, 
        const uint32_t frameNumber );

    virtual Equalizer* clone() const { return new TileEqualizer( *this ); }
    virtual void toStream( std::ostream& os ) const { os << this; }

    void setName( const std::string& name ) { _name = name; }

    void setTileSize( const Vector2i& size ) { _size = size; }

    const std::string& getName() const { return _name; }

    const Vector2i& getTileSize() const { return _size; }

protected:

    virtual void notifyChildAdded( Compound* compound, Compound* child ) {}
    virtual void notifyChildRemove( Compound* compound, Compound* child ) {}

private:
    
    void _syncViewTileSize( Compound* compound );
    void _destroyQueues( Compound* compound );
    void _createQueues( Compound* compound );

    bool _created;
    Vector2i _size;
    std::string _name;
};

} //server
} //eq

#endif // EQS_TILEEQUALIZER_H
