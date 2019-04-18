
/* Copyright (c) 2011-2018, Stefan Eilemann <eile@eyescale.ch>
 *                          Daniel Nachbaur <danielnachbaur@googlemail.com>
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

#include <co/queueMaster.h>
#include <lunchbox/bitOperation.h> // function getIndexOfLastBit

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
    TileQueue(const TileQueue& from);

    /**
     * @name Data Access
     */
    //@{
    void setCompound(Compound* compound)
    {
        LBASSERT(!_compound);
        _compound = compound;
    }
    Compound* getCompound() const { return _compound; }
    Channel* getChannel() const
    {
        return _compound ? _compound->getChannel() : 0;
    }
    Node* getNode() const { return _compound ? _compound->getNode() : 0; }
    void setName(const std::string& name) { _name = name; }
    const std::string& getName() const { return _name; }

    /** Set the size of the tiles. */
    void setTileSize(const Vector2i& size) { _size = size; }

    /** @return the tile size. */
    const Vector2i& getTileSize() const { return _size; }

    /** Set the size of the DB chunk. */
    void setChunkSize(const float size) { _chunkSize = size; }

    /** @return the DB chunk size. */
    float getChunkSize() const { return _chunkSize; }

    /** Add a tile to the queue. */
    void addTile(const Tile& tile, const Eye eye);

    /**
     * Cycle the current tile queue.
     *
     * Used for output tile queues to allocate/recycle queue masters.
     *
     * @param frameNumber the current frame number.
     * @param compound the compound holding the output frame.
     */
    void cycleData(const uint32_t frameNumber, const Compound* compound);

    void setOutputQueue(TileQueue* queue, const Compound* compound);
    const TileQueue* getOutputQueue(const Eye eye) const
    {
        return _outputQueue[lunchbox::getIndexOfLastBit(eye)];
    }

    /**
     * @name Operations
     */
    //@{
    /** Unset the tile data. */
    void unsetData();

    /** Reset the frame and delete all tile datas. */
    void flush();
    //@}

    uint128_t getQueueMasterID(const Eye eye) const;

protected:
    EQSERVER_API virtual ChangeType getChangeType() const { return INSTANCE; }
    EQSERVER_API virtual void getInstanceData(co::DataOStream& os);
    EQSERVER_API virtual void applyInstanceData(co::DataIStream& is);

private:
    struct LatencyQueue
    {
        uint32_t _frameNumber;
        co::QueueMaster _queue;
    };

    /** The parent compound. */
    Compound* _compound;

    /** The name which associates input to output frames. */
    std::string _name;

    /** The size of each tile in the queue. */
    Vector2i _size;

    /** The DB range width of each tile in the queue. */
    float _chunkSize = 1;

    /** The collage queue pool. */
    std::deque<LatencyQueue*> _queues;

    /** the currently used tile queues */
    LatencyQueue* _queueMaster[NUM_EYES];

    /** The current output queue. */
    TileQueue* _outputQueue[NUM_EYES];
};

std::ostream& operator<<(std::ostream& os, const TileQueue* frame);
} // namespace server
} // namespace eq
#endif // EQSERVER_TILEQUEUE_H
