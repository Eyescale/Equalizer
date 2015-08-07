
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010-2013, Stefan Eilemann <eile@eyescale.ch>
 *               2011, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQSERVER_CHANGELATENCYVISITOR_H
#define EQSERVER_CHANGELATENCYVISITOR_H

#include "compound.h"
#include "configVisitor.h"
#include "frame.h"
#include "layout.h"
#include "node.h"
#include "observer.h"
#include "segment.h"
#include "tileQueue.h"
#include "view.h"

namespace eq
{
namespace server
{
/**
 * The Change Latency visitor modifies the latency on all relevant objects.
 */
class ChangeLatencyVisitor : public ConfigVisitor
{
public:
    explicit ChangeLatencyVisitor( const uint32_t latency )
        : _latency( latency ) {}

    virtual VisitorResult visit( Compound* compound )
    {
        const Frames& outputFrames = compound->getOutputFrames();
        for( FramesCIter i = outputFrames.begin();
             i != outputFrames.end(); ++i )
        {
            Frame* frame = *i;
            frame->setAutoObsolete( _latency );
        }

        const Frames& inputFrames = compound->getInputFrames();
        for( FramesCIter i = inputFrames.begin();
             i != inputFrames.end(); ++i )
        {
            Frame* frame = *i;
            frame->setAutoObsolete( _latency );
        }

        const TileQueues& outputTileQueues = compound->getOutputTileQueues();
        for( TileQueuesCIter i = outputTileQueues.begin();
             i != outputTileQueues.end(); ++i )
        {
            TileQueue* queue = *i;
            queue->setAutoObsolete( _latency );
        }

        const TileQueues& inputTileQueues = compound->getInputTileQueues();
        for( TileQueuesCIter i = inputTileQueues.begin();
             i != inputTileQueues.end(); ++i )
        {
            TileQueue* queue = *i;
            queue->setAutoObsolete( _latency );
        }
        return TRAVERSE_CONTINUE;
    }

    virtual VisitorResult visitPre( Node* node )
    {
        node->changeLatency( _latency );
        return TRAVERSE_CONTINUE;
    }

    virtual VisitorResult visit( Observer* observer )
        { return _visit( observer ); }

    virtual VisitorResult visitPre( Layout* layout )
        { return _visit( layout ); }
    virtual VisitorResult visit( View* view )
        { return _visit( view ); }

    virtual VisitorResult visitPre( Segment* segment )
        { return _visit( segment ); }
    virtual VisitorResult visit( Segment* segment )
        { return _visit( segment ); }

private:
    const uint32_t _latency;

    VisitorResult _visit( co::Object* object )
        {
            // double commit on update/delete
            object->setAutoObsolete( _latency + 1 );
            return TRAVERSE_CONTINUE;
        }
};

}
}
#endif // EQSERVER_CHANGELATENCYVISITOR
