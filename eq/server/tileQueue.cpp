
/* Copyright (c) 2011-2014, Stefan Eilemann <eile@eyescale.ch>
 *               2011-2012, Daniel Nachbaur <danielnachbaur@googlemail.com>
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

#include "tileQueue.h"

#include <eq/fabric/tile.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <co/queueItem.h>

namespace eq
{
namespace server
{

TileQueue::TileQueue()
        : co::Object()
        , _compound( 0 )
        , _name()
        , _size( 0, 0 )
{
    for( unsigned i = 0; i < NUM_EYES; ++i )
    {
        _queueMaster[i] = 0;
        _outputQueue[i] = 0;
    }
}

TileQueue::TileQueue( const TileQueue& from )
        : co::Object()
        , _compound( 0 )
        , _name( from._name )
        , _size( from._size )
{
    for( unsigned i = 0; i < NUM_EYES; ++i )
    {
        _queueMaster[i] = 0;
        _outputQueue[i] = 0;
    }
}

TileQueue::~TileQueue()
{
    _compound = 0;
}

void TileQueue::addTile( const Tile& tile, const fabric::Eye eye )
{
    uint32_t index = lunchbox::getIndexOfLastBit(eye);
    LBASSERT( index < NUM_EYES );
    _queueMaster[index]->_queue.push() << tile;
}

void TileQueue::cycleData( const uint32_t frameNumber, const Compound* compound)
{
    for( unsigned i = 0; i < NUM_EYES; ++i )
    {
        if( !compound->isInheritActive( Eye( 1<<i )))// eye pass not used
        {
            _queueMaster[i] = 0;
            continue;
        }

        // reuse unused queues
        LatencyQueue* queue    = _queues.empty() ? 0 : _queues.back();
        const uint32_t latency = getAutoObsolete();
        const uint32_t dataAge = queue ? queue->_frameNumber : 0;

        if( queue && dataAge < frameNumber-latency && frameNumber > latency )
            // not used anymore
            _queues.pop_back();
        else // still used - allocate new data
        {
            queue = new LatencyQueue;

            getLocalNode()->registerObject( &queue->_queue );
            queue->_queue.setAutoObsolete( 1 ); // current + in use by render nodes
        }

        queue->_queue.clear();
        queue->_frameNumber = frameNumber;

        _queues.push_front( queue );
        _queueMaster[i] = queue;
    }
}

void TileQueue::setOutputQueue( TileQueue* queue, const Compound* compound )
{
    for( unsigned i = 0; i < NUM_EYES; ++i )
    {
        // eye pass not used && no output frame for eye pass
        if( compound->isInheritActive( Eye( 1<<i )))
            _outputQueue[i] =queue;
    }
}

void TileQueue::getInstanceData( co::DataOStream& )
{
}

void TileQueue::applyInstanceData( co::DataIStream& )
{
}

void TileQueue::flush()
{
    unsetData();

    while( !_queues.empty( ))
    {
        LatencyQueue* queue = _queues.front();
        _queues.pop_front();
        getLocalNode()->deregisterObject( &queue->_queue );
        delete queue;
    }

}

void TileQueue::unsetData()
{
    for( unsigned i = 0; i < NUM_EYES; ++i )
    {
        _queueMaster[i] = 0;
        _outputQueue[i] = 0;
    }
}

uint128_t TileQueue::getQueueMasterID( const Eye eye ) const
{
    uint32_t index = lunchbox::getIndexOfLastBit(eye);
    LatencyQueue* queue = _queueMaster[ index ];
    if ( queue )
        return queue->_queue.getID();
    return uint128_t();
}

std::ostream& operator << ( std::ostream& os, const TileQueue* tileQueue )
{
    if( !tileQueue )
        return os;

    os << lunchbox::disableFlush << "tiles" << std::endl;
    os << "{" << std::endl << lunchbox::indent;

    const std::string& name = tileQueue->getName();
    os << "name      \"" << name << "\"" << std::endl;

    const Vector2i& size = tileQueue->getTileSize();
    if( size != Vector2i::ZERO )
        os << "size      " << size << std::endl;

    os << lunchbox::exdent << "}" << std::endl << lunchbox::enableFlush;
    return os;
}

}
}
