
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.h> 
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

#include <eq/client/configParams.h>
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

bool Resources::discover( Config* config, const std::string& session,
                          const uint32_t flags )
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
    gpusd::GPUInfos infos = gpusd::Module::discoverGPUs( filter );

    if( infos.empty( ))
    {
        EQINFO << "No resources found for session " << session 
               << ", using default config" << std::endl;
        infos.push_back( gpusd::GPUInfo( ));
    }
    typedef stde::hash_map< std::string, Node* > NodeMap;

    NodeMap nodes;
    const bool multiprocess = flags & ConfigParams::FLAG_MULTIPROCESS;

    size_t nodeCounter = 0;
    size_t gpuCounter = 0;
    for( gpusd::GPUInfosCIter i = infos.begin(); i != infos.end(); ++i )
    {
        const gpusd::GPUInfo& info = *i;

        Node* node = nodes[ info.hostname ];
        if( !node || multiprocess )
        {
            const bool isApplicationNode = info.hostname.empty() && !node;
            node = new Node( config );

            std::stringstream nodeName;
            nodeName << info.hostname << "." << ++nodeCounter;

            node->setName( nodeName.str() );
            node->setHost( info.hostname );
            node->setApplicationNode( isApplicationNode );

            co::ConnectionDescriptionPtr desc = new ConnectionDescription;
            desc->setHostname( info.hostname );
            node->addConnectionDescription( desc );

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

    Node* node = config->findAppNode();
    if( !node )
    {
        node = new Node( config );
        node->setApplicationNode( true );
        node->addConnectionDescription( new ConnectionDescription );
    }
    if( node->getPipes().empty( )) // add display window
    {
        Pipe* pipe = new Pipe( node );
        pipe->setName( "display" );
    }

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

        _addMonoCompound( segmentCompound, channels );
        _addStereoCompound( segmentCompound, channels );
    }
}

Compound* Resources::_addMonoCompound( Compound* root, const Channels& channels )
{
    const Channel* channel = root->getChannel();
    const Layout* layout = channel->getLayout();
    const std::string& name = layout->getName();

    Compound* compound = 0;

    if( name == EQ_SERVER_CONFIG_LAYOUT_SIMPLE )
        /* nop */;
    else if( name == EQ_SERVER_CONFIG_LAYOUT_2D_DYNAMIC ||
        name == EQ_SERVER_CONFIG_LAYOUT_2D_STATIC )
    {
        compound = _add2DCompound( root, channels );
    }
    else if( name == EQ_SERVER_CONFIG_LAYOUT_DB_DYNAMIC ||
        name == EQ_SERVER_CONFIG_LAYOUT_DB_STATIC )
    {
        compound = _addDBCompound( root, channels );
    }
    else if( name == EQ_SERVER_CONFIG_LAYOUT_DB_DS )
    {
        compound = _addDSCompound( root, channels );
    }
    else
    {
        EQASSERTINFO( false, "Unimplemented mode " << name );
    }

    if( !compound )
        return 0;

    compound->setEyes( EYE_CYCLOP );
    return compound;
}

Compound* Resources::_addStereoCompound( Compound* root,
                                         const Channels& channels )
{
    const Channel* channel = root->getChannel();
    const Layout* layout = channel->getLayout();
    const std::string& name = layout->getName();
    if( name == EQ_SERVER_CONFIG_LAYOUT_SIMPLE )
        return 0;

    Compound* compound = new Compound( root );
    compound->setName( "Stereo" );
    compound->setEyes( EYE_LEFT | EYE_RIGHT );

    const size_t nChannels = channels.size();
    const ChannelsCIter split = channels.begin() + (nChannels >> 1);

    Channels leftChannels( split - channels.begin( ));
    std::copy( channels.begin(), split, leftChannels.begin( ));

    Channels rightChannels( channels.end() - split );
    std::copy( split, channels.end(), rightChannels.begin( ));

    Compound* left = 0;
    if( leftChannels.empty() ||
        ( leftChannels.size() == 1 && leftChannels.front() == channel ))
    {
        left = new Compound( compound );
    }
    else
        left = _addMonoCompound( compound, leftChannels );

    left->setEyes( EYE_LEFT );

    Compound* right = 0;
    if( rightChannels.empty() ||
        ( rightChannels.size() == 1 && rightChannels.front() == channel ))
    {
        right = new Compound( compound );
    }
    else
        right = _addMonoCompound( compound, rightChannels );

    right->setEyes( EYE_RIGHT );

    return compound;
}

Compound* Resources::_add2DCompound( Compound* root, const Channels& channels )
{
    const Channel* channel = root->getChannel();
    const Layout* layout = channel->getLayout();
    const std::string& name = layout->getName();

    Compound* compound = new Compound( root );
    compound->setName( name );
    if( name == EQ_SERVER_CONFIG_LAYOUT_2D_DYNAMIC )
        compound->addEqualizer( new LoadEqualizer( LoadEqualizer::MODE_2D ));

    const Compounds& children = _addSources( compound, channels );
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
    const Channel* channel = root->getChannel();
    const Layout* layout = channel->getLayout();
    const std::string& name = layout->getName();

    Compound* compound = new Compound( root );
    compound->setName( name );
    compound->setBuffers( eq::Frame::BUFFER_COLOR|eq::Frame::BUFFER_DEPTH );
    if( name == EQ_SERVER_CONFIG_LAYOUT_DB_DYNAMIC )
        compound->addEqualizer( new LoadEqualizer( LoadEqualizer::MODE_DB ));

    const Compounds& children = _addSources( compound, channels );
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

Compound* Resources::_addDSCompound( Compound* root, const Channels& channels )
{
    const Channel* channel = root->getChannel();
    const Layout* layout = channel->getLayout();
    const std::string& name = layout->getName();

    Compound* compound = new Compound( root );
    compound->setName( name );

    const Compounds& children = _addSources( compound, channels );

    const size_t step = size_t( 100000.0f / float( children.size( )));

    size_t start = 0;
    for( CompoundsCIter i = children.begin(); i != children.end(); ++i )
    {
        Compound* child = *i;

        // leaf draw + tile readback compound
        Compound* drawChild = new Compound( child );
        if( i+1 == children.end( ) ) // last - correct rounding 'error'
        {
            drawChild->setRange(
                eq::Range( static_cast< float >( start )/100000.f, 1.f ));
        }
        else
            drawChild->setRange(
                eq::Range( static_cast< float >( start )/100000.f,
                           static_cast< float >( start + step )/100000.f ));
        
        unsigned y = 0;
        for( CompoundsCIter j = children.begin(); j != children.end(); ++j )
        {
            if( i != j )
            {
                std::ostringstream frameName;
                frameName << "tile" << j - children.begin() << ".channel"
                          << i - children.begin();
                eq::Viewport vp;
                if(  j+1 == children.end( ) ) // last - correct rounding 'error'
                {
                    vp = eq::Viewport( 0.f, static_cast< float >( y )/100000.f,
                              1.f, static_cast< float >( 100000-y )/100000.f );
                }
                else
                    vp = eq::Viewport( 0.f, static_cast< float >( y )/100000.f,
                                  1.f, static_cast< float >( step )/100000.f );

                eq::server::Frame* outputFrame = _createFrame( frameName, vp );
                outputFrame->setBuffers( eq::Frame::BUFFER_COLOR |
                                         eq::Frame::BUFFER_DEPTH );
                drawChild->addOutputFrame( outputFrame );

                // input tiles from other channels
                frameName.str("");
                frameName << "tile" << i - children.begin() << ".channel"
                          << j - children.begin();

                child->addInputFrame( _createFrame( frameName ));
            }
            // else own tile, is in place

            y += step;
        }
 
        // assembled color tile output, if not already in place
        if( i != children.begin( ) )
        {
            std::ostringstream frameName;
            frameName << "frame.channel" << i - children.begin();

            Viewport vp;
            if( i+1 == children.end( ) ) // last - correct rounding 'error'
            {
                vp = eq::Viewport( 0.f, static_cast< float >( start )/100000.f,
                                  1.f,
                               static_cast< float >( 100000-start )/100000.f );
            }
            else
                vp = eq::Viewport( 0.f, static_cast< float >( start )/100000.f,
                                  1.f, static_cast< float >( step )/100000.f );

            child->addOutputFrame(   _createFrame( frameName, vp, true ));
            compound->addInputFrame( _createFrame( frameName ));
        }
        start += step;
    }

    return compound;
}

const Compounds& Resources::_addSources( Compound* compound,
                                         const Channels& channels )
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

    return compound->getChildren();
}

eq::server::Frame* Resources::_createFrame( const char* name )
{
  eq::server::Frame* frame = new eq::server::Frame;
  frame->setName( std::string( name ));
  return frame;
}

eq::server::Frame* Resources::_createFrame( std::ostringstream& name )
{
  return _createFrame( name.str().c_str( ));
}

eq::server::Frame* Resources::_createFrame( std::ostringstream& name,
                                       const eq::fabric::Viewport& vp,
                                       bool colorOnly )
{
  eq::server::Frame* frame = _createFrame( name );
  frame->setViewport( vp );
  if( colorOnly )
      frame->setBuffers( eq::Frame::BUFFER_COLOR );
  return frame;
}

}
}
}
