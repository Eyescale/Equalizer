
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
#include "../layout.h"
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

bool Resources::discoverLocal( Config* config )
{
    const GPUInfos infos = UIFactory::discoverGPUs();
    if( infos.empty( ))
        return false;

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
    return true;
}

Channels Resources::configureSourceChannels( Config* config )
{
    Channels channels;

    const Node* node = config->findAppNode();
    EQASSERT( node );
    if( !node )
        return channels;

    const Pipes& pipes = node->getPipes();
    EQASSERT( !pipes.empty( ));
    if( pipes.empty( ))
        return channels;

    Pipe* pipe = pipes.front();
    PixelViewport pvp = pipe->getPixelViewport();
    if( pvp.isValid( ))
    {
        pvp.x = 0;
        pvp.y = 0;
    }
    else
        pvp = PixelViewport( 0, 0, 1920, 1200 );

    if( pipe->getName() != "display" ) // add as resource
        channels.push_back( pipe->getChannel( ChannelPath( 0 )));

    for( PipesCIter i = ++pipes.begin(); i != pipes.end(); ++i )
    {
        pipe = *i;
        Window* window = new Window( pipe );
        window->setPixelViewport( pvp );
        window->setIAttribute( Window::IATTR_HINT_DRAWABLE, fabric::FBO );
        window->setName( pipe->getName() + " source window" );

        channels.push_back( new Channel( window ));
        channels.back()->setName( pipe->getName() + " source channel" );
    }

    return channels;
}

void Resources::configure( const Compounds& compounds, const Channels& channels)
{
    EQASSERT( compounds.size() == 1 );
    if( compounds.empty() || channels.empty()) // No additional resources
        return;

    const Canvas* canvas = 0;
    const Compounds& children = compounds.front()->getChildren();
    for( CompoundsCIter i = children.begin(); i != children.end(); ++i )
    {
        Compound* segmentCompound = *i;
        const Channel* channel = segmentCompound->getChannel();
        EQASSERT( channel );

        EQASSERT( !canvas || channel->getCanvas() == canvas );
        canvas = channel->getCanvas();

        const Layout* layout = channel->getLayout();
        EQASSERT( layout );
        
        const std::string& name = layout->getName();
        if( name == "2D" )
        {
            Compound* mono = _add2DCompound( segmentCompound, channels );
            mono->setEyes( EYE_CYCLOP );

            Compound* stereo =_addEyeCompound( segmentCompound, channels );
            stereo->setEyes( EYE_LEFT | EYE_RIGHT );
        }
        else if( name == "Simple" )
            /* nop */ ;
        else
        {
            EQASSERTINFO( 0, "Unimplemented" );
        }
    }
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
