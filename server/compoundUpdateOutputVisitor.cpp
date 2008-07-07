
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compoundUpdateOutputVisitor.h"

#include "frame.h"
#include "frameData.h"
#include "swapBarrier.h"

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

Compound::VisitorResult CompoundUpdateOutputVisitor::visitLeaf(
    Compound* compound )
{
    _updateOutput( compound );
    _updateSwapBarriers( compound );

    return Compound::TRAVERSE_CONTINUE;    
}

void CompoundUpdateOutputVisitor::_updateOutput( Compound* compound )
{
    const std::vector< Frame* >& outputFrames = compound->getOutputFrames();
    const Channel*               channel      = compound->getChannel();

    if( !compound->testInheritTask( Compound::TASK_READBACK ) || 
        outputFrames.empty( ) || !channel )

        return;

    for( vector<Frame*>::const_iterator i = outputFrames.begin(); 
         i != outputFrames.end(); ++i )
    {
        Frame*             frame  = *i;
        const std::string& name   = frame->getName();

        if( _outputFrames.find( name ) != _outputFrames.end())
        {
            EQWARN << "Multiple output frames of the same name are unsupported"
                   << ", ignoring output frame " << name << endl;
            frame->unsetData();
            continue;
        }

        const eq::Viewport& frameVP = frame->getViewport();
        const eq::PixelViewport& inheritPVP=compound->getInheritPixelViewport();
        eq::PixelViewport framePVP = inheritPVP.getSubPVP( frameVP );

        // FrameData offset is position wrt destination view
        frame->cycleData( _frameNumber, compound->getInheritEyes( ));
        FrameData* frameData = frame->getMasterData();
        EQASSERT( frameData );

        frameData->setOffset( vmml::Vector2i( framePVP.x, framePVP.y ));

        EQLOG( eq::LOG_ASSEMBLY )
            << disableFlush << "Output frame \"" << name << "\" id " 
            << frame->getID() << " v" << frame->getVersion()+1
            << " data id " << frameData->getID() << " v" 
            << frameData->getVersion() + 1 << " on channel \""
            << channel->getName() << "\" tile pos " << framePVP.x << ", " 
            << framePVP.y;

        // FrameData pvp is area within channel
        framePVP.x = (int32_t)(frameVP.x * inheritPVP.w);
        framePVP.y = (int32_t)(frameVP.y * inheritPVP.h);
        frameData->setPixelViewport( framePVP );

        // Frame offset is position wrt window, i.e., the channel position
        if( compound->getInheritChannel() == channel
            /* || use dest channel origin hint set */ )

            frame->setOffset( vmml::Vector2i( inheritPVP.x, inheritPVP.y ));
        else
        {
            const eq::PixelViewport& nativePVP = channel->getPixelViewport();
            frame->setOffset( vmml::Vector2i( nativePVP.x, nativePVP.y ));
        }

        // image buffers
        uint32_t buffers = frame->getBuffers();
        frameData->setBuffers( buffers == eq::Frame::BUFFER_UNDEFINED ? 
                                   compound->getInheritBuffers() : buffers );

        // (source) render context
        frameData->setRange( compound->getInheritRange( ));
        frameData->setPixel( compound->getInheritPixel( ));

        frame->commitData();
        frame->updateInheritData( compound );
        frame->commit();
        _outputFrames[name] = frame;

        EQLOG( eq::LOG_ASSEMBLY ) 
            << " buffers frame " << frame->getInheritBuffers() << " data " 
            << frameData->getBuffers() << " read area " << framePVP << endl
            << enableFlush;
    }
}

void CompoundUpdateOutputVisitor::_updateSwapBarriers( Compound* compound )
{
    const SwapBarrier* swapBarrier = compound->getSwapBarrier();
    if( !swapBarrier )
        return;

    Window* window = compound->getWindow();
    if( !window )
        return;

    const std::string& barrierName = swapBarrier->getName();
    hash_map<string, eq::net::Barrier*>::const_iterator iter = 
        _swapBarriers.find( barrierName );

    if( iter == _swapBarriers.end( ))
        _swapBarriers[barrierName] = window->newSwapBarrier();
    else
        window->joinSwapBarrier( iter->second );
}

}
}
