
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "compoundUpdateInputVisitor.h"

#include "frame.h"
#include "frameData.h"
#include "tileQueue.h"
#include "server.h"
#include "node.h"

#include <eq/log.h>
#include <eq/fabric/iAttribute.h>

namespace eq
{
namespace server
{
CompoundUpdateInputVisitor::CompoundUpdateInputVisitor(
    const Compound::FrameMap& outputFrames,
    const Compound::TileQueueMap& outputQueues )
        : _outputFrames( outputFrames )
        , _outputQueues( outputQueues )
{}

VisitorResult CompoundUpdateInputVisitor::visit( Compound* compound )
{
    if( !compound->isRunning( ))
        return TRAVERSE_PRUNE;    
    
    const TileQueues& inputQueues = compound->getInputTileQueues();
    const Channel* channel = compound->getChannel();

    if( !compound->testInheritTask( fabric::TASK_ASSEMBLE ) || !channel )
        return TRAVERSE_CONTINUE;

    for( TileQueuesCIter i = inputQueues.begin(); i != inputQueues.end(); ++i )
    {
        //----- Find corresponding output queue
        TileQueue* queue  = *i;
        const std::string& name = queue->getName();

        Compound::TileQueueMap::const_iterator j = _outputQueues.find( name );

        if( j == _outputQueues.end( ))
        {
            EQVERB << "Can't find matching output queue, ignoring input queue "
                << name << std::endl;
            queue->unsetData();
            continue;
        }

        EQASSERT( queue->isAttached( ));

        TileQueue* outputQueue = j->second;
        queue->addOutputQueue( outputQueue, compound );
    }

    const Frames& inputFrames = compound->getInputFrames();
    if( inputFrames.empty( ))
    {
        compound->unsetInheritTask( fabric::TASK_ASSEMBLE );
        return TRAVERSE_CONTINUE;
    }

    for( FramesCIter i = inputFrames.begin(); i != inputFrames.end(); ++i )
    {
        //----- Find corresponding output frame
        Frame* frame = *i;
        const std::string& name = frame->getName();

        Compound::FrameMap::const_iterator j = _outputFrames.find( name );

        if( j == _outputFrames.end( ))
        {
            EQVERB << "Can't find matching output frame, ignoring input frame "
                   << name << std::endl;
            frame->unsetData();
            continue;
        }

        //----- Set frame parameters:
        // 1) Frame offset
        Frame* outputFrame = j->second;
        const Channel* iChannel = compound->getInheritChannel();
        Vector2i frameOffset = outputFrame->getMasterData()->getOffset() +
                               frame->getOffset();

        if( outputFrame->getCompound()->getInheritChannel() != iChannel )
            frameOffset = frame->getOffset();
        else if( channel != iChannel )
        {
            // compute delta offset between source and destination, since the
            // channel's native origin (as opposed to destination) is used.
            const Viewport& frameVP = frame->getViewport();
            const PixelViewport& inheritPVP=compound->getInheritPixelViewport();
            PixelViewport framePVP( inheritPVP );

            framePVP.apply( frameVP );
            frameOffset.x() -= framePVP.x;
            frameOffset.y() -= framePVP.y;

            const PixelViewport& iChannelPVP = iChannel->getPixelViewport();
            frameOffset.x() -= iChannelPVP.x;
            frameOffset.y() -= iChannelPVP.y;
        }
        frame->setInheritOffset( frameOffset );

        // 2) zoom
        _updateZoom( compound, frame, outputFrame );

        // 3) TODO input frames are moved using the offset. The pvp signifies
        //    the pixels to be used from the frame data.
        //framePVP.x = static_cast< int32_t >( frameVP.x * inheritPVP.w );
        //framePVP.y = static_cast< int32_t >( frameVP.y * inheritPVP.h );
        //frame->setInheritPixelViewport( framePVP );
        //----- Link input frame to output frame (connects frame data)
        outputFrame->addInputFrame( frame, compound );

        for( unsigned k = 0; k < NUM_EYES; ++k )
        {
            const eq::Eye eye = eq::Eye( 1<<k );
            if( compound->isInheritActive( eye ) &&  // eye pass used
                outputFrame->hasData( eye ))         // output data for eye pass
            {
                frame->commit();
                EQLOG( LOG_ASSEMBLY )
                    << "Input frame  \"" << name << "\" on channel \"" 
                    << channel->getName() << "\" id " << frame->getID() << " v"
                    << frame->getVersion() << "\" tile pos "
                    << frame->getInheritOffset() << ' '
                    << frame->getInheritZoom() << std::endl;
                break;
            }
        }
    }

    for( Compound::FrameMap::const_iterator i = _outputFrames.begin(); i != _outputFrames.end(); ++i )
    {
        Frame* frame  = i->second;
        fabric::Eye eye = fabric::EYE_CYCLOP;
        for ( ; eye < fabric::EYES_ALL; eye = fabric::Eye(eye<<1) )
        {
            const Frames& inputFrames = frame->getInputFrames( eye );
            std::vector< uint128_t > nodeIDs;
            std::vector< uint128_t > netNodeIDs;

            for( FramesCIter j = inputFrames.begin();
                j != inputFrames.end(); ++j )
            {
                const Frame* inputFrame   = *j;
                const Node*  inputNode    = inputFrame->getNode();
                co::NodePtr inputNetNode = inputNode->getNode();
                netNodeIDs.push_back( inputNetNode->getNodeID() );   
                nodeIDs.push_back( inputNode->getID() );
            }

            FrameData* frameData = frame->getMasterData();
            EQASSERT( frameData );
            frameData->setInputNodes( eye, nodeIDs, netNodeIDs );
        }
    }

    return TRAVERSE_CONTINUE;
}

void CompoundUpdateInputVisitor::_updateZoom( const Compound* compound,
                                              Frame* frame, 
                                              const Frame* outputFrame )
{
    Zoom zoom = frame->getZoom();
    if( !zoom.isValid( )) // if zoom is not set, inherit from parent
        zoom = compound->getInheritZoom();

    // Zoom difference between output and input
    const FrameData* frameData = outputFrame->getMasterData();
    zoom /= frameData->getZoom();

    frame->setInheritZoom( zoom );
}

}
}
