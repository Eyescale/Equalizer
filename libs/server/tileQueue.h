
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>
 *               2011, Daniel Nachbaur <danielnachbaur@googlemail.com>
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

#ifndef EQSERVER_TILEQUEUE_H
#define EQSERVER_TILEQUEUE_H

#include "compound.h"
#include "types.h"

#include <co/base/bitOperation.h> // function getIndexOfLastBit
#include <co/queueMaster.h>

namespace eq
{
namespace server
{
    /** A holder for tile data and parameters. */
    class TileQueue : public co::Object
    {
    public:
        /** 
         * Constructs a new TileQueue.
         */
        EQSERVER_API TileQueue();
        EQSERVER_API virtual ~TileQueue();
        TileQueue( const TileQueue& from );

        /**
         * @name Data Access
         */
        //@{
        void setCompound( Compound* compound )
            { EQASSERT( !_compound ); _compound = compound; }
        Compound* getCompound() const { return _compound; }
        Channel* getChannel() const
            { return _compound ? _compound->getChannel() :0; }
        Node* getNode() const { return _compound ? _compound->getNode() : 0; }

        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        co::ObjectVersion getDataVersion( const Eye eye ) const;

        /** Set the size of the tiles. */
        void setTileSize( const Vector2i& size ) { _size = size; }

        /** @return the tile size. */
        const Vector2i& getTileSize() const { return _size; }

        /** Add a tile to the queue. */
        void addTile( const TileTaskPacket& tile );

        /**
         * @name Operations
         */
        //@{
        /** Unset the tile data. */
        void unsetData();

        /** Reset the frame and delete all tile datas. */
        void flush();
        //@}

    protected:
        EQSERVER_API virtual ChangeType getChangeType() const 
                                                            { return INSTANCE; }
        EQSERVER_API virtual void getInstanceData( co::DataOStream& os );
        EQSERVER_API virtual void applyInstanceData( co::DataIStream& is );

    private:
        /** The parent compound. */
        Compound* _compound;

        /** The name which associates input to output frames. */
        std::string _name;

        /** The size of each tile in the queue. */
        Vector2i _size;

        /** The collage tile queue holding the tiles. */
        co::QueueMaster _queueMaster;
    };

    std::ostream& operator << ( std::ostream& os, const TileQueue* frame );
}
}
#endif // EQSERVER_TILEQUEUE_H
