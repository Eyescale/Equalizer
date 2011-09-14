
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_COMPOUNDUPDATEOUTPUTVISITOR_H
#define EQSERVER_COMPOUNDUPDATEOUTPUTVISITOR_H

#include "compoundVisitor.h" // base class
#include "compound.h"        // nested type

namespace eq
{
namespace server
{

    enum TileStrategy
    {
        STGY_TILE_RASTER,
        STGY_TILE_ZIGZAG,
        STGY_TILE_CWSPIRAL,
        STGY_TILE_SQSPIRAL,
        STGY_TILE_CUSTOM=10
    };

   
    /**
     * The compound visitor updating the output data (frames, tiles,
     * swapbarriers) of a compound tree.
     */
    class CompoundUpdateOutputVisitor : public CompoundVisitor
    {
    public:
        CompoundUpdateOutputVisitor( const uint32_t frameNumber );
        virtual ~CompoundUpdateOutputVisitor() {}

        /** Visit all compounds. */
        virtual VisitorResult visit( Compound* compound );

        const Compound::BarrierMap& getSwapBarriers() const
            { return _swapBarriers; }
        const Compound::FrameMap& getOutputFrames() const
            { return _outputFrames; }
        const Compound::TileQueueMap& getOutputQueues() const
            { return _outputTileQueues; }

    private:
        const uint32_t _frameNumber;
 
        Compound::BarrierMap   _swapBarriers;
        Compound::FrameMap     _outputFrames;
        Compound::TileQueueMap _outputTileQueues;

        void _updateOutput( Compound* compound );
        void _updateZoom( const Compound* compound, Frame* frame );
        void _updateSwapBarriers( Compound* compound );

        void _generateTiles( TileQueue* queue, 
                             Compound* compound,
                             TileStrategy strategy );
        
        void _addTilesToQueue( TileQueue* queue, 
                               Compound* compound, 
                               std::vector< Vector2i >& tileOrder );

        void _applyTileStrategy( std::vector< Vector2i >& tileOrder,
                                 const Vector2i& dim,
                                 TileStrategy strategy );

        void _rasterStrategy( std::vector< Vector2i >& tileOrder,
                              const Vector2i& dim );

        void _zigzagStrategy( std::vector< Vector2i >& tileOrder,
                              const Vector2i& dim );

        void _spiralStrategy( std::vector< Vector2i >& tileOrder,
                              const Vector2i& dim );

        void _squareStrategy( std::vector< Vector2i >& tileOrder,
                              const Vector2i& dim );
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
