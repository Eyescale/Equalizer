
/* Copyright (c) 2008-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "equalizer.h"

namespace eq
{
namespace server
{

std::ostream& operator << ( std::ostream& os, const TileEqualizer* );

class TileEqualizer : public Equalizer
{
public:
    EQSERVER_API TileEqualizer();
    TileEqualizer( const TileEqualizer& from );
    ~TileEqualizer() {}

    /** @sa CompoundListener::notifyUpdatePre */
    void notifyUpdatePre( Compound* compound,
                          const uint32_t frameNumber ) final;

    void toStream( std::ostream& os ) const final { os << this; }
    void setName( const std::string& name ) { _name = name; }

    const std::string& getName() const { return _name; }

    uint32_t getType() const final { return fabric::TILE_EQUALIZER; }

protected:
    void notifyChildAdded( Compound*, Compound* ) override {}
    void notifyChildRemove( Compound*, Compound* ) override {}

private:
    std::string _getQueueName() const;
    void _destroyQueues( Compound* compound );
    void _createQueues( Compound* compound );

    bool _created;
    std::string _name;
};

} //server
} //eq

#endif // EQS_TILEEQUALIZER_H
