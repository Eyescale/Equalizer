
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
#include "../configVisitor.h"
#include "../connectionDescription.h"
#include "../frame.h"
#include "../layout.h"
#include "../node.h"
#include "../pipe.h"
#include "../segment.h"
#include "../window.h"
#include "../equalizers/loadEqualizer.h"

#include <eq/client/frame.h>
#include <eq/client/windowSystem.h>
#include <eq/fabric/gpuInfo.h>

#include <gpusd/gpuInfo.h>
#include <gpusd/module.h>
#ifdef EQ_USE_GPUSD_cgl
#  include <gpusd/cgl/module.h>
#endif
#ifdef EQ_USE_GPUSD_glx
#  include <gpusd/glx/module.h>
#endif
#ifdef EQ_USE_GPUSD_wgl
#  include <gpusd/wgl/module.h>
#endif
#ifdef EQ_USE_GPUSD_dns_sd
#  include <gpusd/dns_sd/module.h>
#endif

namespace eq
{
namespace server
{
namespace config
{
static co::base::a_int32_t _frameCounter;

bool Resources::discover( Config* config, const std::string& session )
{
#ifdef EQ_USE_GPUSD_cgl
    gpusd::cgl::Module::use();
#elif defined(EQ_USE_GPUSD_glx)
    gpusd::glx::Module::use();
#endif
#ifdef EQ_USE_GPUSD_wgl
    gpusd::wgl::Module::use();
#endif
#ifdef EQ_USE_GPUSD_dns_sd
    gpusd::dns_sd::Module::use();
#endif

    gpusd::FilterPtr filter = *gpusd::FilterPtr( new gpusd::MirrorFilter ) |
                               gpusd::FilterPtr( new gpusd::DuplicateFilter );
    if( !session.empty( ))
        *filter |= gpusd::FilterPtr( new gpusd::SessionFilter( session ));
    const gpusd::GPUInfos& infos = gpusd::Module::discoverGPUs( filter );

    if( infos.empty( ))
    {
        EQINFO << "No resources found for session " << session << std::endl;
        return false;
    }
    typedef stde::hash_map< std::string, Node* > NodeMap;

    NodeMap nodes;
    Node* node = new Node( config ); // Add default appNode
    node->setName( "Local Node" );
    node->setApplicationNode( true );
    nodes[ "" ] = node;

    size_t gpuCounter = 0;
    for( gpusd::GPUInfosCIter i = infos.begin(); i != infos.end(); ++i )
    {
        const gpusd::GPUInfo& info = *i;

        node = nodes[ info.hostname ];
        if( !node )
        {
            node = new Node( config );
            node->setName( info.hostname );
            node->setHost( info.hostname );
            nodes[ info.hostname ] = node;
        }

        Pipe* pipe = new Pipe( node );
        pipe->setPort( info.port );
        pipe->setDevice( info.device );
        pipe->setPixelViewport( PixelViewport( info.pvp ));

        std::stringstream name;
        if( info.device == EQ_UNDEFINED_UINT32 )
            name << "display";
        else
            name << "GPU" << ++gpuCounter;

        pipe->setName( name.str( ));
    }

    node = nodes[ "" ];
    if( node->getPipes().empty( )) // add display window
    {
        Pipe* pipe = new Pipe( node );
        pipe->setName( "display" );
    }
    if( nodes.size() > 1 ) // add appNode connection for cluster configs
        node->addConnectionDescription( new ConnectionDescription );

    return true;
}

namespace
{
class AddSourcesVisitor : public ConfigVisitor
{
public:
    AddSourcesVisitor( const PixelViewport& pvp ) : _pvp( pvp ) {}

    virtual VisitorResult visitPre( Pipe* pipe )
        {
            const Node* node = pipe->getNode();
            if( node->isApplicationNode() && node->getPipes().front() == pipe )
            {
                // display window has discrete 'affinity' GPU
                if( pipe->getName() != "display" )
                    _channels.push_back( pipe->getChannel( ChannelPath( 0 )));
                return TRAVERSE_CONTINUE;
            }

            Window* window = new Window( pipe );
            if( !pipe->getPixelViewport().isValid( ))
                window->setPixelViewport( _pvp );
            window->setIAttribute( Window::IATTR_HINT_DRAWABLE, fabric::FBO );
            window->setName( pipe->getName() + " source window" );

            _channels.push_back( new Channel( window ));
            _channels.back()->setName( pipe->getName() + " source channel" );
            return TRAVERSE_CONTINUE; 
        }

    const Channels& getChannels() const { return _channels; }
private:
    const PixelViewport& _pvp;
    Channels _channels;
};
}

Channels Resources::configureSourceChannels( Config* config )
{
    const Node* node = config->findAppNode();
    EQASSERT( node );
    if( !node )
        return Channels();

    const Pipes& pipes = node->getPipes();
    EQASSERT( !pipes.empty( ));
    if( pipes.empty( ))
        return Channels();

    Pipe* pipe = pipes.front();
    PixelViewport pvp = pipe->getPixelViewport();
    if( pvp.isValid( ))
    {
        pvp.x = 0;
        pvp.y = 0;
    }
    else
        pvp = PixelViewport( 0, 0, 1920, 1200 );

    AddSourcesVisitor addSources( pvp );
    config->accept( addSources );
    return addSources.getChannels();
}

#if 0 // EQ_GCC_4_5_OR_LATER
#  pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
void Resources::configure( const Compounds& compounds, const Channels& channels)
{
    EQASSERT( !compounds.empty( ));
    if( compounds.empty() || channels.empty()) // No additional resources
        return;

    const Canvas* canvas = 0;
    for( CompoundsCIter i = compounds.begin(); i != compounds.end(); ++i )
    {
        const Compounds& children = (*i)->getChildren();
        EQASSERT( children.size() == 1 );
        if( children.size() != 1 )
            continue;

        Compound* segmentCompound = children.front();
        const Channel* channel = segmentCompound->getChannel();
        EQASSERT( channel );

        EQASSERT( !canvas || channel->getCanvas() == canvas );
        canvas = channel->getCanvas();

        const Layout* layout = channel->getLayout();
        EQASSERT( layout );
        
        const std::string& name = layout->getName();
        if( name == "Static 2D" || name == "Dynamic 2D" )
        {
            Compound* mono = _add2DCompound( segmentCompound, channels );
            mono->setEyes( EYE_CYCLOP );

            Compound* stereo =_addEyeCompound( segmentCompound, channels );
            stereo->setEyes( EYE_LEFT | EYE_RIGHT );
            if( name == "Dynamic 2D" )
            {
                mono->addEqualizer( new LoadEqualizer( LoadEqualizer::MODE_2D));
                stereo->addEqualizer(new LoadEqualizer(LoadEqualizer::MODE_2D));
            }
        }
        else if( name == "Static DB" || name == "Dynamic DB" )
        {
            Compound* db = _addDBCompound( segmentCompound, channels );
            db->setName( name );
            if( name == "Dynamic DB" )
                db->addEqualizer( new LoadEqualizer( LoadEqualizer::MODE_DB ));
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
    _addSources( compound, channels );

    const Compounds& children = compound->getChildren();
    const size_t step =  size_t( 100000.0f / float( children.size( )));
    size_t start = 0;
    for( CompoundsCIter i = children.begin(); i != children.end(); ++i )
    {
        Compound* child = *i;
        if( i+1 == children.end( )) // last - correct rounding 'error'
            child->setViewport(
                fabric::Viewport( float( start ) / 100000.f, 0.f,
                                  ( 100000.f - float( start ))/100000.f, 1.f ));
        else
            child->setViewport(
                fabric::Viewport( float( start ) / 100000.f, 0.f,
                                  float( step ) / 100000.f, 1.f ));
        start += step;
    }

    return compound;
}

Compound* Resources::_addDBCompound( Compound* root, const Channels& channels )
{
    Compound* compound = new Compound( root );
    compound->setName( "DB" );
    if( channels.size() > 1 )
        compound->setBuffers( eq::Frame::BUFFER_COLOR|eq::Frame::BUFFER_DEPTH );
    _addSources( compound, channels );

    const Compounds& children = compound->getChildren();
    const size_t step = size_t( 100000.0f / float( children.size( )));
    size_t start = 0;
    for( CompoundsCIter i = children.begin(); i != children.end(); ++i )
    {
        Compound* child = *i;
        if( i+1 == children.end( )) // last - correct rounding 'error'
            child->setRange( Range( float( start ) / 100000.f, 1.f ));
        else
            child->setRange( Range( float( start ) / 100000.f,
                                    float( start + step ) / 100000.f ));
        start += step;
    }
    return compound;
}

void Resources::_addSources( Compound* compound, const Channels& channels )
{
    const Channel* rootChannel = compound->getChannel();
    const Segment* segment = rootChannel->getSegment();
    const Channel* outputChannel = segment ? segment->getChannel() : 0;
    EQASSERT( outputChannel );

    for( ChannelsCIter i = channels.begin(); i != channels.end(); ++i )
    {
        Channel* channel = *i;
        Compound* child = new Compound( compound );

        if( channel == outputChannel )
            continue;
        child->setChannel( channel );

        Frame* outFrame = new Frame;
        std::stringstream frameName;
        frameName << "Frame." << compound->getName() << '.' << ++_frameCounter;
        outFrame->setName( frameName.str( ));
        child->addOutputFrame( outFrame );

        Frame* inFrame = new Frame;
        inFrame->setName( frameName.str( ));
        compound->addInputFrame( inFrame );
    }
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
