
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compoundUpdateInputVisitor.h"

#include "frame.h"
#include "frameData.h"

#include <eq/client/log.h>

using namespace std;
using namespace stde;
using namespace eq::base;

namespace eq
{
namespace server
{
CompoundUpdateInputVisitor::CompoundUpdateInputVisitor(
    const stde::hash_map<std::string, Frame*>& outputFrames )
        : _outputFrames( outputFrames )
{}

VisitorResult CompoundUpdateInputVisitor::visitLeaf(
    Compound* compound )
{
    const std::vector< Frame* >& inputFrames = compound->getInputFrames();
    const Channel*               channel     = compound->getChannel();

    if( !compound->testInheritTask( eq::TASK_ASSEMBLE ) || !channel )
        return TRAVERSE_CONTINUE;

    if( inputFrames.empty( ))
    {
        compound->unsetInheritTask( eq::TASK_ASSEMBLE );
        return TRAVERSE_CONTINUE;
    }

    for( vector<Frame*>::const_iterator i = inputFrames.begin(); 
         i != inputFrames.end(); ++i )
    {
        Frame*                             frame = *i;
        const std::string&                 name = frame->getName();
        Compound::FrameMap::const_iterator iter =_outputFrames.find(name);

        if( iter == _outputFrames.end())
        {
            EQWARN << "Can't find matching output frame, ignoring input frame "
                   << name << endl;
            frame->unsetData();
            continue;
        }

        Frame*          outputFrame = iter->second;
        const eq::Viewport& frameVP = frame->getViewport();
        const eq::PixelViewport& inheritPVP=compound->getInheritPixelViewport();
        eq::PixelViewport  framePVP = inheritPVP.getSubPVP( frameVP );
        vmml::Vector2i  frameOffset = outputFrame->getMasterData()->getOffset();

        if( channel != compound->getInheritChannel() &&
            compound->getIAttribute( Compound::IATTR_HINT_OFFSET ) != eq::ON )
        {
            // compute delta offset between source and destination, since the
            // channel's native origin (as opposed to destination) is used.
            frameOffset.x -= framePVP.x;
            frameOffset.y -= framePVP.y;
        }

        // input frames are moved using the offset. The pvp signifies the pixels
        // to be used from the frame data.
        framePVP.x = static_cast<int32_t>( frameVP.x * inheritPVP.w );
        framePVP.y = static_cast<int32_t>( frameVP.y * inheritPVP.h );

        frame->setOffset( frameOffset );
        //frame->setPixelViewport( framePVP );
        outputFrame->addInputFrame( frame, compound->getInheritEyes( ));
        frame->updateInheritData( compound );
        frame->commit();

        EQLOG( eq::LOG_ASSEMBLY )
            << "Input frame  \"" << name << "\" on channel \"" 
            << channel->getName() << "\" id " << frame->getID() << " v"
            << frame->getVersion() << " buffers " << frame->getInheritBuffers() 
            << "\" tile pos " << frameOffset << " sub-pvp " << framePVP << endl;
    }

    return TRAVERSE_CONTINUE;
}

}
}
