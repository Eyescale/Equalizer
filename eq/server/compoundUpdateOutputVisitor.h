
/* Copyright (c) 2007-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "compound.h"        // nested type
#include "compoundVisitor.h" // base class

namespace eq
{
namespace server
{
/**
 * The compound visitor updating the output data (frames, tiles,
 * swapbarriers) of a compound tree.
 */
class CompoundUpdateOutputVisitor : public CompoundVisitor
{
public:
    explicit CompoundUpdateOutputVisitor(const uint32_t frameNumber);
    virtual ~CompoundUpdateOutputVisitor() {}
    /** Visit all compounds. */
    virtual VisitorResult visit(Compound* compound);

    const Compound::BarrierMap& getSwapBarriers() const
    {
        return _swapBarriers;
    }
    const Compound::FrameMap& getOutputFrames() const { return _outputFrames; }
    const Compound::TileQueueMap& getOutputQueues() const
    {
        return _outputTileQueues;
    }

private:
    const uint32_t _frameNumber;

    Compound::BarrierMap _swapBarriers;
    Compound::FrameMap _outputFrames;
    Compound::TileQueueMap _outputTileQueues;

    void _updateQueues(Compound* compound);
    void _updateFrames(Compound* compound);
    void _updateSwapBarriers(Compound* compound);
    void _updateZoom(const Compound* compound, Frame* frame);

    void _generateTiles(TileQueue* queue, Compound* compound);
    void _addTilesToQueue(TileQueue* queue, Compound* compound,
                          const std::vector<Vector2i>& tiles);
};
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
