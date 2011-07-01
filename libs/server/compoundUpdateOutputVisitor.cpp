
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

#include "compoundUpdateOutputVisitor.h"

#include "frame.h"
#include "frameData.h"
#include "swapBarrier.h"
#include "window.h"
#include "tileQueue.h"
#include "server.h"
#include "config.h"

#include <eq/log.h>
#include <eq/fabric/iAttribute.h>

namespace eq
{
namespace server
{
CompoundUpdateOutputVisitor::CompoundUpdateOutputVisitor(  
    const uint32_t frameNumber )
        : _frameNumber( frameNumber )
{}

VisitorResult CompoundUpdateOutputVisitor::visit( Compound* compound )
{
    if( !compound->isRunning( ))
        return TRAVERSE_PRUNE;    

    _updateOutput( compound );
    _updateSwapBarriers( compound );

    return TRAVERSE_CONTINUE;    
}

void CompoundUpdateOutputVisitor::_updateOutput( Compound* compound )
{
    const Channel* channel = compound->getChannel();

    if( !compound->testInheritTask( fabric::TASK_READBACK ) || !channel )
        return;

    const TileQueues& outputQueues = compound->getOutputTileQueues();
    for( TileQueuesCIter i = outputQueues.begin(); 
        i != outputQueues.end(); ++i )
    {
        //----- Check uniqueness of output queue name
        TileQueue* queue  = *i;
        if (!queue->isActivated())
            continue;

        const std::string& name   = queue->getName();

        if( _outputTileQueues.find( name ) != _outputTileQueues.end())
        {
            EQWARN << "Multiple output queues of the same name are unsupported"
                << ", ignoring output queue " << name << std::endl;
            queue->unsetData();
            continue;
        }

        queue->cycleData( _frameNumber, compound );

        //----- Generate tile task packets
        _generateTiles( queue, compound );

        _outputTileQueues[name] = queue;
    }

    const Frames& outputFrames = compound->getOutputFrames();
    if( outputFrames.empty( ))
    {
        compound->unsetInheritTask( fabric::TASK_READBACK );
        return;
    }

    for( Frames::const_iterator i = outputFrames.begin(); 
         i != outputFrames.end(); ++i )
    {
        //----- Check uniqueness of output frame name
        Frame*             frame  = *i;
        const std::string& name   = frame->getName();

        if( _outputFrames.find( name ) != _outputFrames.end())
        {
            EQWARN << "Multiple output frames of the same name are unsupported"
                   << ", ignoring output frame " << name << std::endl;
            frame->unsetData();
            continue;
        }

        //----- compute readback area
        const Viewport& frameVP = frame->getViewport();
        const PixelViewport& inheritPVP = compound->getInheritPixelViewport();
        PixelViewport framePVP( inheritPVP );
        framePVP.apply( frameVP );
        
        if( !framePVP.hasArea( )) // output frame has no pixels
        {
            EQINFO << "Skipping output frame " << name << ", no pixels"
                   << std::endl;
            frame->unsetData();
            continue;
        }

        //----- Create new frame datas
        //      one frame data used for each eye pass
        //      data is set only on master frame data (will copy to all others)
        frame->cycleData( _frameNumber, compound );
        FrameData* frameData = frame->getMasterData();
        EQASSERT( frameData );

        EQLOG( LOG_ASSEMBLY )
            << co::base::disableFlush << "Output frame \"" << name << "\" id " 
            << frame->getID() << " v" << frame->getVersion()+1
            << " data id " << frameData->getID() << " v" 
            << frameData->getVersion() + 1 << " on channel \""
            << channel->getName() << "\" tile pos " << framePVP.x << ", " 
            << framePVP.y;

        //----- Set frame data parameters:
        // 1) offset is position wrt destination view
        frameData->setOffset( Vector2i( framePVP.x, framePVP.y ));

        // 2) pvp is area within channel
        framePVP.x = static_cast< int32_t >( frameVP.x * inheritPVP.w );
        framePVP.y = static_cast< int32_t >( frameVP.y * inheritPVP.h );
        frameData->setPixelViewport( framePVP );

        // 3) image buffers and storage type
        uint32_t buffers = frame->getBuffers();

        frameData->setType( frame->getType() );
        frameData->setBuffers( buffers == eq::Frame::BUFFER_UNDEFINED ? 
                                   compound->getInheritBuffers() : buffers );

        // 4) (source) render context
        frameData->setRange( compound->getInheritRange( ));
        frameData->setPixel( compound->getInheritPixel( ));
        frameData->setSubPixel( compound->getInheritSubPixel( ));
        frameData->setPeriod( compound->getInheritPeriod( ));
        frameData->setPhase( compound->getInheritPhase( ));

        //----- Set frame parameters:
        // 1) offset is position wrt window, i.e., the channel position
        if( compound->getInheritChannel() == channel )
            frame->setInheritOffset( Vector2i( inheritPVP.x, inheritPVP.y ));
        else
        {
            const PixelViewport& nativePVP = channel->getPixelViewport();
            frame->setInheritOffset( Vector2i( nativePVP.x, nativePVP.y ));
        }

        // 2) zoom
        _updateZoom( compound, frame );

        _outputFrames[name] = frame;
        EQLOG( LOG_ASSEMBLY ) 
            << " buffers " << frameData->getBuffers() << " read area "
            << framePVP << " readback " << frame->getInheritZoom()
            << " assemble " << frameData->getZoom() << std::endl
            << co::base::enableFlush;
    }
}

void CompoundUpdateOutputVisitor::_generateTiles( TileQueue* queue,
                                                  Compound* compound )
{
    const Vector2i& tileSize = queue->getTileSize();
    PixelViewport pvp = compound->getInheritPixelViewport();
    if( !pvp.hasArea( ))
        return;

    double xFraction = 1.0 / pvp.w;
    double yFraction = 1.0 / pvp.h;
    float vpWidth = tileSize.x() * xFraction;
    float vpHeight = tileSize.y() * yFraction;

    for (int32_t x = 0; x < pvp.w; x += tileSize.x())
    {
        for (int32_t y = 0; y < pvp.h; y += tileSize.y())
        {
            PixelViewport tilepvp;
            Viewport tilevp;

            tilepvp.x = x;
            tilepvp.y = y;
            tilevp.x = x * xFraction;
            tilevp.y = y * yFraction;

            if ( x + tileSize.x() <= pvp.w )
            {
                tilepvp.w = tileSize.x();
                tilevp.w = vpWidth;
            }
            else
            {
                // no full tile
                tilepvp.w = pvp.w  - x;
                tilevp.w = tilepvp.w * xFraction;
            }

            if ( y + tileSize.y() <= pvp.h )
            {
                tilepvp.h = tileSize.y();
                tilevp.h = vpHeight;
            }
            else
            {
                // no full tile
                tilepvp.h = pvp.h - y;
                tilevp.h = tilepvp.h * yFraction;
            }

            fabric::Eye eye = fabric::EYE_CYCLOP;
            for ( ; eye < fabric::EYES_ALL; eye = fabric::Eye(eye<<1) )
            {
                if ( !(compound->getInheritEyes() & eye) ||
                     !compound->isInheritActive( eye ))
                    continue;

                TileTaskPacket packet;
                packet.pvp = tilepvp;
                packet.vp = tilevp;

                compound->computeTileFrustum( packet.frustum, 
                                              eye, packet.vp, false );
                compound->computeTileFrustum( packet.ortho, 
                                              eye, packet.vp, true );
                queue->addTile( packet, eye );
            }
        }
    }
}

void CompoundUpdateOutputVisitor::_updateZoom( const Compound* compound,
                                               Frame* frame )
{
    Zoom zoom = frame->getZoom();
    Zoom zoom_1;

    if( !zoom.isValid( )) // if zoom is not set, auto-calculate from parent
    {
        zoom_1 = compound->getInheritZoom();
        EQASSERT( zoom_1.isValid( ));
        zoom.x() = 1.0f / zoom_1.x();
        zoom.y() = 1.0f / zoom_1.y();
    }
    else
    {
        zoom_1.x() = 1.0f / zoom.x();
        zoom_1.y() = 1.0f / zoom.y();
    }

    if( frame->getType( ) == eq::Frame::TYPE_TEXTURE )
    {
        FrameData* frameData = frame->getMasterData();
        frameData->setZoom( zoom_1 ); // textures are zoomed by input frame
        frame->setInheritZoom( Zoom::NONE );
    }
    else
    {
        Zoom inputZoom;
        /* Output frames downscale pixel data during readback, and upscale it on
         * the input frame by setting the input frame's inherit zoom. */
        if( zoom.x() > 1.0f )
        {
            inputZoom.x() = zoom_1.x();
            zoom.x()      = 1.f;
        }
        if( zoom.y() > 1.0f )
        {
            inputZoom.y() = zoom_1.y();
            zoom.y()      = 1.f;
        }

        FrameData* frameData = frame->getMasterData();
        frameData->setZoom( inputZoom );
        frame->setInheritZoom( zoom );                
    }
}

void CompoundUpdateOutputVisitor::_updateSwapBarriers( Compound* compound )
{
    const SwapBarrier* swapBarrier = compound->getSwapBarrier();
    if( !swapBarrier )
        return;

    Window* window = compound->getWindow();
    EQASSERT( window );
    if( !window )
        return;

    if( swapBarrier->isNvSwapBarrier( ))
    {
        if( !window->hasNVSwapBarrier( ))
        {
            const std::string name( "__NV_swap_group_protection_barrier__" );
            _swapBarriers[name] = 
                window->joinNVSwapBarrier( swapBarrier, _swapBarriers[name] );
        }
    }
    else
    {
        const std::string& name = swapBarrier->getName();
        _swapBarriers[name] = window->joinSwapBarrier( _swapBarriers[name] );
    }
}

}
}
