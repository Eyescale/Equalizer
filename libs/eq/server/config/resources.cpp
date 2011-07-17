
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.h> 
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

#include "resources.h"

#include "../compound.h"
#include "../frame.h"
#include "../node.h"
#include "../pipe.h"
#include "../window.h"
#include "../equalizers/loadEqualizer.h"

#include <eq/client/uiFactory.h>
#include <eq/fabric/gpuInfo.h>

namespace eq
{
namespace server
{
namespace config
{
static co::base::a_int32_t _frameCounter;

void Resources::discoverLocal( Config* config )
{
    const GPUInfos infos = UIFactory::discoverGPUs();
    EQASSERT( !infos.empty( ));

    Node* node = new Node( config );
    node->setApplicationNode( true );

    size_t gpuCounter = 0;
    for( GPUInfosCIter i = infos.begin(); i != infos.end(); ++i )
    {
        const GPUInfo& info = *i;

        Pipe* pipe = new Pipe( node );
        pipe->setPort( info.port );
        pipe->setDevice( info.device );
        pipe->setPixelViewport( info.pvp );

        std::stringstream name;
        if( info.device == EQ_UNDEFINED_UINT32 )
            name << "display";
        else
            name << "GPU" << ++gpuCounter;

        pipe->setName( name.str( ));
    }
}

void Resources::configure( const Compounds& compounds )
{
    EQASSERT( compounds.size() == 1 );
    if( compounds.empty( ))
        return;

    const Compounds& segmentCompounds = compounds.front()->getChildren();
    EQASSERT( segmentCompounds.size() == 1 );
    if( segmentCompounds.empty( ))
        return;

    Compound* segmentCompound = segmentCompounds.front();
    const Config* config = segmentCompound->getConfig();
    const Node* node = config->findAppNode();
    EQASSERT( node );
    if( !node )
        return;

    const Pipes& pipes = node->getPipes();
    EQASSERT( !pipes.empty( ));
    if( pipes.empty( ))
        return;

    Channel* segmentChannel = segmentCompound->getChannel();
    EQASSERT( segmentChannel );
    if( !segmentChannel )
        return;

    PipesCIter i = pipes.begin();
    const Pipe* displayPipe = *i;
    PixelViewport pvp = displayPipe->getPixelViewport();
    if( pvp.isValid( ))
    {
        pvp.x = 0;
        pvp.y = 0;
    }
    else
        pvp = PixelViewport( 0, 0, 1920, 1200 );

    Channels channels;
    for( ++i; i != pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        Window* window = new Window( pipe );
        window->setPixelViewport( pvp );
        window->setIAttribute( Window::IATTR_HINT_DRAWABLE, fabric::FBO );
        window->setName( pipe->getName() + " source window" );

        channels.push_back( new Channel( window ));
        channels.back()->setName( pipe->getName() + " source channel" );
    }
    if( channels.empty( )) // No additional resources
        return;

    if( displayPipe->getName() != "display" )
        channels.push_back( segmentChannel );

    Compound* monoCompound = _add2DCompound( segmentCompound, channels );
    monoCompound->setEyes( EYE_CYCLOP );

    Compound* stereoCompound = _addEyeCompound(segmentCompound, channels);
    stereoCompound->setEyes( EYE_LEFT | EYE_RIGHT );
}

Compound* Resources::_add2DCompound( Compound* root, const Channels& channels )
{
    Compound* compound = new Compound( root );
    compound->setName( "2D" );
    compound->addEqualizer( new LoadEqualizer );

    const Channel* rootChannel = compound->getChannel();
    for( ChannelsCIter i = channels.begin(); i != channels.end(); ++i )
    {
        Channel* channel = *i;
        Compound* child = new Compound( compound );
        child->setChannel( channel );

        if( channel == rootChannel )
            continue;

        Frame* outFrame = new Frame;
        std::stringstream frameName;
        frameName << "Frame.2D." << ++_frameCounter;
        outFrame->setName( frameName.str( ));
        child->addOutputFrame( outFrame );

        Frame* inFrame = new Frame;
        inFrame->setName( frameName.str( ));
        compound->addInputFrame( inFrame );
    }
    return compound;
}

Compound* Resources::_addEyeCompound( Compound* root, const Channels& channels )
{
    Compound* compound = new Compound( root );
    compound->setName( "Stereo" );

    const size_t nChannels = channels.size();
    const ChannelsCIter split = channels.begin() + (nChannels >> 1);

    Channels leftChannels( split - channels.begin( ));
    std::copy( channels.begin(), split, leftChannels.begin( ));

    Channels rightChannels( channels.end() - (split+1));
    std::copy( split+1, channels.end(), rightChannels.begin( ));
    
    const Channel* rootChannel = compound->getChannel();

    Compound* left = 0;
    if( leftChannels.empty() ||
        ( leftChannels.size() == 1 && leftChannels.front() == rootChannel ))
    {
        left = new Compound( compound );
    }
    else
        left = _add2DCompound( compound, leftChannels );

    left->setEyes( EYE_LEFT | EYE_CYCLOP );

    Compound* right = 0;
    if( rightChannels.empty() ||
        ( rightChannels.size() == 1 && rightChannels.front() == rootChannel ))
    {
        right = new Compound( compound );
    }
    else
        right = _add2DCompound( compound, rightChannels );

    right->setEyes( EYE_RIGHT | EYE_CYCLOP );

    return compound;
}

}
}
}
