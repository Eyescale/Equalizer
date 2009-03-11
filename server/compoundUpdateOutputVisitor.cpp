
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compoundUpdateOutputVisitor.h"

#include "frame.h"
#include "frameData.h"
#include "swapBarrier.h"
#include "window.h"

#include <eq/client/log.h>

using namespace std;
using namespace stde;
using namespace eq::base;

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
    if( !compound->isActive( ))
        return TRAVERSE_PRUNE;    

    _updateOutput( compound );
    _updateSwapBarriers( compound );

    return TRAVERSE_CONTINUE;    
}

void CompoundUpdateOutputVisitor::_updateOutput( Compound* compound )
{
    const std::vector< Frame* >& outputFrames = compound->getOutputFrames();
    const Channel*               channel      = compound->getChannel();

    if( !compound->testInheritTask( eq::TASK_READBACK ) || !channel )
        return;

    if( outputFrames.empty( ))
    {
        compound->unsetInheritTask( eq::TASK_READBACK );
        return;
    }

    for( vector<Frame*>::const_iterator i = outputFrames.begin(); 
         i != outputFrames.end(); ++i )
    {
        //----- Check uniqueness of output frame name
        Frame*             frame  = *i;
        const std::string& name   = frame->getName();

        if( _outputFrames.find( name ) != _outputFrames.end())
        {
            EQWARN << "Multiple output frames of the same name are unsupported"
                   << ", ignoring output frame " << name << endl;
            frame->unsetData();
            continue;
        }

        //----- compute readback area
        const eq::Viewport& frameVP = frame->getViewport();
        const eq::PixelViewport& inheritPVP=compound->getInheritPixelViewport();
        eq::PixelViewport framePVP = inheritPVP.getSubPVP( frameVP );
        
        if( !framePVP.hasArea( )) // output frame has no pixels
        {
            EQINFO << "Skipping output frame " << name << ", no pixels" << endl;
            frame->unsetData();
            continue;
        }

        //----- Create new frame datas
        //      one frame data used for each eye pass
        //      data is set only on master frame data (will copy to all others)
        frame->cycleData( _frameNumber, compound->getInheritEyes( ));
        FrameData* frameData = frame->getMasterData();
        EQASSERT( frameData );

        EQLOG( eq::LOG_ASSEMBLY )
            << disableFlush << "Output frame \"" << name << "\" id " 
            << frame->getID() << " v" << frame->getVersion()+1
            << " data id " << frameData->getID() << " v" 
            << frameData->getVersion() + 1 << " on channel \""
            << channel->getName() << "\" tile pos " << framePVP.x << ", " 
            << framePVP.y;

        //----- Set frame data parameters:
        // 1) offset is position wrt destination view
        frameData->setOffset( vmml::Vector2i( framePVP.x, framePVP.y ));

        // 2) pvp is area within channel
        framePVP.x = (int32_t)(frameVP.x * inheritPVP.w);
        framePVP.y = (int32_t)(frameVP.y * inheritPVP.h);
        frameData->setPixelViewport( framePVP );

        // 3) image buffers and storage type
        uint32_t buffers = frame->getBuffers();

        frameData->setType( frame->getType() );
        frameData->setBuffers( buffers == eq::Frame::BUFFER_UNDEFINED ? 
                                   compound->getInheritBuffers() : buffers );

        // 4) (source) render context
        frameData->setRange( compound->getInheritRange( ));
        frameData->setPixel( compound->getInheritPixel( ));

        //----- Set frame parameters:
        // 1) offset is position wrt window, i.e., the channel position
        if( compound->getInheritChannel() == channel ||
            compound->getIAttribute( Compound::IATTR_HINT_OFFSET ) == eq::ON )
        {
            frame->setInheritOffset(
                vmml::Vector2i( inheritPVP.x, inheritPVP.y ));
        }
        else
        {
            const eq::PixelViewport& nativePVP = channel->getPixelViewport();
            frame->setInheritOffset(
                vmml::Vector2i( nativePVP.x, nativePVP.y ));
        }

        // 2) zoom
        _updateZoom( compound, frame );

        //----- Commit
        frame->commitData();
        frame->commit();

        _outputFrames[name] = frame;
        EQLOG( eq::LOG_ASSEMBLY ) 
            << " buffers " << frameData->getBuffers() << " read area "
            << framePVP << " readback " << frame->getInheritZoom()
            << " assemble " << frameData->getZoom() << endl << enableFlush;
    }
}

void CompoundUpdateOutputVisitor::_updateZoom( const Compound* compound,
                                               Frame* frame )
{
    eq::Zoom zoom = frame->getZoom();
    eq::Zoom zoom_1;

    if( !zoom.isValid( )) // if zoom is not set, auto-calculate from parent
    {
        zoom_1 = compound->getInheritZoom();
        EQASSERT( zoom_1.isValid( ));
        zoom.x = 1.0f / zoom_1.x;
        zoom.y = 1.0f / zoom_1.y;
    }
    else
    {
        zoom_1.x = 1.0f / zoom.x;
        zoom_1.y = 1.0f / zoom.y;
    }

    if( frame->getType( ) == eq::Frame::TYPE_TEXTURE )
    {
        FrameData* frameData = frame->getMasterData();
        frameData->setZoom( zoom_1 ); // textures are zoomed by input frame
        frame->setInheritZoom( eq::Zoom( ));
    }
    else
    {
        eq::Zoom inputZoom;
        /* Output frames downscale pixel data during readback, and upscale it on
         * the input frame by setting the input frame's inherit zoom. */
        if( zoom.x > 1.0f )
        {
            inputZoom.x = zoom_1.x;
            zoom.x      = 1.f;
        }
        if( zoom.y > 1.0f )
        {
            inputZoom.y = zoom_1.y;
            zoom.y      = 1.f;
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

    if( swapBarrier->isNvSwapBarrier() )
        return;
    
    Window* window = compound->getWindow();
    if( !window )
        return;

    const std::string& barrierName = swapBarrier->getName();
    Compound::BarrierMap::const_iterator i = _swapBarriers.find( barrierName );

    if( i == _swapBarriers.end( ))
        _swapBarriers[barrierName] = window->newSwapBarrier();
    else
        window->joinSwapBarrier( i->second );
}

}
}
