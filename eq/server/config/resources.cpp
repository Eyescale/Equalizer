
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.h>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Julio Delgado Mangas <julio.delgadomangas@epfl.ch>
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

#include <eq/fabric/configParams.h>
#include <eq/fabric/gpuInfo.h>

#include <hwsd/gpuInfo.h>
#include <hwsd/netInfo.h>
#include <hwsd/hwsd.h>
#ifdef EQUALIZER_USE_hwsd_gpu_cgl
#  include <hwsd/gpu/cgl/module.h>
#endif
#ifdef EQUALIZER_USE_hwsd_gpu_glx
#  include <hwsd/gpu/glx/module.h>
#endif
#ifdef EQUALIZER_USE_hwsd_gpu_wgl
#  include <hwsd/gpu/wgl/module.h>
#endif
#ifdef EQUALIZER_USE_hwsd_gpu_dns_sd
#  include <hwsd/gpu/dns_sd/module.h>
#endif
#ifdef EQUALIZER_USE_hwsd_net_sys
#  include <hwsd/net/sys/module.h>
#endif
#ifdef EQUALIZER_USE_hwsd_net_dns_sd
#  include <hwsd/net/dns_sd/module.h>
#endif

#ifdef _MSC_VER
#  include <eq/os.h>
#  define setenv( name, value, overwrite ) \
    _putenv_s( name, value )
#endif

#define USE_IPv4

namespace eq
{
namespace server
{
namespace config
{
namespace
{
co::ConnectionDescriptions _findConnections( const lunchbox::uint128_t& id,
                                             const hwsd::NetInfos& netInfos )
{
    // sort connections by bandwidth
    typedef std::multimap< int32_t, co::ConnectionDescriptionPtr > Connections;
    Connections connections;

    for( hwsd::NetInfosCIter i = netInfos.begin(); i != netInfos.end(); ++i )
    {
        const hwsd::NetInfo& netInfo = *i;
        if( netInfo.id != id || !netInfo.up ||
            netInfo.type == hwsd::NetInfo::TYPE_LOOPBACK )
        {
            continue;
        }

        co::ConnectionDescriptionPtr desc = new ConnectionDescription;
        switch( netInfo.type )
        {
            case hwsd::NetInfo::TYPE_ETHERNET:
                desc->type = co::CONNECTIONTYPE_TCPIP;
                desc->bandwidth = 125000;   // 1Gbit
                break;

            case hwsd::NetInfo::TYPE_INFINIBAND:
                desc->type = co::CONNECTIONTYPE_RDMA;
                desc->bandwidth = 2500000;   // 20Gbit
                break;

            default:
                desc->type = co::CONNECTIONTYPE_NONE;
        }
        if( netInfo.linkspeed != hwsd::NetInfo::defaultValue )
            desc->bandwidth = netInfo.linkspeed * 125; // MBit -> Kbyte
#ifdef USE_IPv4
        desc->hostname = netInfo.inetAddress;
#else
        desc->hostname = netInfo.inet6Address;
#endif
        connections.insert( std::make_pair( desc->bandwidth, desc ));
    }

    co::ConnectionDescriptions result;
    for( Connections::const_reverse_iterator i = connections.rbegin();
         i != connections.rend(); ++i )
    {
        result.push_back( i->second );
    }
    return result;
}

uint32_t _configureNetworkTypes( const fabric::ConfigParams& params )
{
    uint32_t netTypes = 0;
    if( params.getFlags() & fabric::ConfigParams::FLAG_NETWORK_ALL )
    {
        if( params.getFlags() & fabric::ConfigParams::FLAG_NETWORK_ETHERNET )
            netTypes |= hwsd::NetInfo::TYPE_ETHERNET;
        if( params.getFlags() & fabric::ConfigParams::FLAG_NETWORK_INFINIBAND )
            netTypes |= hwsd::NetInfo::TYPE_INFINIBAND;
    }
    else
        netTypes = hwsd::NetInfo::TYPE_ALL ^ hwsd::NetInfo::TYPE_UNKNOWN;

    return netTypes;
}

hwsd::GPUInfos _discoverGPUs( const fabric::ConfigParams& params,
                              hwsd::FilterPtr filter )
{
    hwsd::FilterPtr gpuFilter = filter | new hwsd::MirrorFilter |
                                new hwsd::GPUFilter( params.getGPUFilter( ));

    return hwsd::discoverGPUInfos( gpuFilter );
}

hwsd::NetInfos _discoverNetworks( const fabric::ConfigParams& params,
                                  hwsd::FilterPtr filter )
{

    hwsd::FilterPtr netFilter = filter |
                          new hwsd::NetFilter( params.getPrefixes(),
                                              _configureNetworkTypes( params ));

    return hwsd::discoverNetInfos( netFilter );
}

void _configureHwsdModules()
{
#ifdef EQUALIZER_USE_hwsd_gpu_cgl
    hwsd::gpu::cgl::Module::use();
#elif defined(EQUALIZER_USE_hwsd_gpu_glx)
    hwsd::gpu::glx::Module::use();
#endif
#ifdef EQUALIZER_USE_hwsd_gpu_wgl
    hwsd::gpu::wgl::Module::use();
#endif
#ifdef EQUALIZER_USE_hwsd_gpu_dns_sd
    hwsd::gpu::dns_sd::Module::use();
#endif
#ifdef EQUALIZER_USE_hwsd_net_sys
    hwsd::net::sys::Module::use();
#endif
#ifdef EQUALIZER_USE_hwsd_net_dns_sd
    hwsd::net::dns_sd::Module::use();
#endif
}

void _disposeHwsdModules()
{
#ifdef EQUALIZER_USE_hwsd_gpu_cgl
    hwsd::gpu::cgl::Module::dispose();
#elif defined(EQUALIZER_USE_hwsd_gpu_glx)
    hwsd::gpu::glx::Module::dispose();
#endif
#ifdef EQUALIZER_USE_hwsd_gpu_wgl
    hwsd::gpu::wgl::Module::dispose();
#endif
#ifdef EQUALIZER_USE_hwsd_gpu_dns_sd
    hwsd::gpu::dns_sd::Module::dispose();
#endif
#ifdef EQUALIZER_USE_hwsd_net_sys
    hwsd::net::sys::Module::dispose();
#endif
#ifdef EQUALIZER_USE_hwsd_net_dns_sd
    hwsd::net::dns_sd::Module::dispose();
#endif
}

} // unnamed namespace

static lunchbox::a_int32_t _frameCounter;

bool Resources::discover( ServerPtr server, Config* config,
                          const std::string& session,
                          const fabric::ConfigParams& params )
{
    _configureHwsdModules();

    hwsd::FilterPtr filter = hwsd::FilterPtr( new hwsd::DuplicateFilter ) |
                             new hwsd::SessionFilter( session );

    hwsd::GPUInfos gpuInfos = _discoverGPUs( params, filter );
    const hwsd::NetInfos& netInfos = _discoverNetworks( params, filter );

    _disposeHwsdModules();

    if( gpuInfos.empty( ))
    {
        if( hwsd::NodeInfo::isLocal( session ))
        {
            LBWARN << "No local GPUs found, abort configuration" << std::endl;
            return false;
        }
        LBINFO << "No resources found for session " << session
               << ", aborting" << std::endl;
        return false;
    }

    typedef stde::hash_map< uint128_t, Node* > NodeMap;
    NodeMap nodes;

    const uint32_t flags = params.getFlags();

    const bool multiProcess = flags & (fabric::ConfigParams::FLAG_MULTIPROCESS |
                                   fabric::ConfigParams::FLAG_MULTIPROCESS_DB );
    const bool multiNode = !hwsd::NodeInfo::isLocal( session ) ||
                           ( multiProcess && gpuInfos.size() > 1 );
    size_t gpuCounter = 0;
    uint128_t appNodeID;

    std::string excludedDisplays; // for VGL_EXCLUDE

    for( const hwsd::GPUInfo& info : gpuInfos )
    {
        if( info.flags & hwsd::GPUInfo::FLAG_VIRTUALGL_DISPLAY )
            continue; // ignore, default $DISPLAY gpu uses this one

        Node* mtNode = nodes[ info.id ];
        Node* mpNode = 0;
        if( !mtNode )
        {
            const bool isApplicationNode = info.nodeName.empty();
            if( isApplicationNode )
                appNodeID = info.id;
            mtNode = new Node( config );
            mtNode->setName( info.nodeName );
            mtNode->setHost( info.nodeName );
            mtNode->setApplicationNode( isApplicationNode );

            nodes[ info.id ] = mtNode;

            if( multiNode )
            {
                const co::ConnectionDescriptions& descs =
                    _findConnections( info.id, netInfos );

                if( descs.empty() && !info.nodeName.empty())
                {
                    LBINFO << "No suitable connection found for node "
                           << info.nodeName << "; node will not be used"
                           << std::endl;
                    nodes.erase( info.id );
                    delete mtNode;
                    continue;
                }

                for( co::ConnectionDescriptionsCIter j = descs.begin();
                     j != descs.end(); ++j )
                {
                    mtNode->addConnectionDescription( *j );
                }
            }
        }
        else if( multiProcess )
        {
            mpNode = new Node( config );
            mpNode->setName( info.nodeName );
            mpNode->setHost( info.nodeName );

            LBASSERT( multiNode );
            const co::ConnectionDescriptions& descs =
                _findConnections( info.id, netInfos );

            if( descs.empty() && !info.nodeName.empty())
            {
                LBINFO << "No suitable connection found for node "
                       << info.nodeName << "; node will not be used"
                       << std::endl;
                delete mpNode;
                continue;
            }

            for( co::ConnectionDescriptionsCIter j = descs.begin();
                 j != descs.end(); ++j )
            {
                mpNode->addConnectionDescription( *j );
            }
        }

        std::stringstream name;
        if( info.device == LB_UNDEFINED_UINT32 &&
            // VirtualGL display redirects to local GPU (see continue above)
            !(info.flags & hwsd::GPUInfo::FLAG_VIRTUALGL ))
        {
            name << "display";
        }
        else
        {
            name << "GPU" << ++gpuCounter;
            if( // When running under VirtualGL, GPUs that are not VNC virtual
                // devices mustn't be interposed.
                ( info.flags & ( hwsd::GPUInfo::FLAG_VIRTUALGL |
                                 hwsd::GPUInfo::FLAG_VNC )) ==
                    hwsd::GPUInfo::FLAG_VIRTUALGL &&
                info.device != LB_UNDEFINED_UINT32 )
            {
                std::ostringstream displayString;
                displayString << ":" << info.port << "." << info.device;
                excludedDisplays += displayString.str() + ",";
            }
        }

        if( mpNode ) // multi-process resource
        {
            Pipe* pipe = new Pipe( mpNode );
            pipe->setPort( info.port );
            pipe->setDevice( info.device );
            pipe->setPixelViewport( PixelViewport( info.pvp ));
            pipe->setName( name.str() + " mp" );
            name << " mt"; // mark companion GPU as multi-threaded only
        }
        else
            name << " mt mp"; // mark GPU as multi-threaded and multi-process

        Pipe* pipe = new Pipe( mtNode ); // standalone/multi-threaded resource
        pipe->setPort( info.port );
        pipe->setDevice( info.device );
        pipe->setPixelViewport( PixelViewport( info.pvp ));
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

    if( config->getNodes().size() > 1 ) // add server connection for clusters
    {
        co::Connections connections;

        if( appNodeID == 0 )
        {
            co::ConnectionDescriptionPtr desc = new co::ConnectionDescription;
            co::ConnectionPtr connection = server->addListener( desc );
            LBASSERT( connection );
            if( connection )
                connections.push_back( connection );
            else
                LBWARN << "Could not add listener " << desc->hostname
                       << " to server" << std::endl;
        }
        else
        {
            const co::ConnectionDescriptions& descs =
                _findConnections( appNodeID, netInfos );

            for( co::ConnectionDescriptionsCIter i = descs.begin();
                 i != descs.end(); ++i )
            {
                co::ConnectionPtr connection = server->addListener( *i );
                LBASSERT( connection );
                if( connection )
                    connections.push_back( connection );
                else
                    LBWARN << "Could not add listener " << (*i)->hostname
                           << " to server" << std::endl;
            }
        }

        config->setServerConnections( connections );
    }

    if( !excludedDisplays.empty( ))
        ::setenv("VGL_EXCLUDE", excludedDisplays.c_str(), 1 /*overwrite*/ );

    return true;
}

namespace
{
class AddSourcesVisitor : public ConfigVisitor
{
public:
    explicit AddSourcesVisitor( const PixelViewport& pvp ) : _pvp( pvp ) {}

    virtual VisitorResult visitPre( Pipe* pipe )
    {
        const Node* node = pipe->getNode();
        if( node->isApplicationNode() && node->getPipes().front() == pipe )
        {
            // display window has discrete 'affinity' GPU
            if( pipe->getName() != "display mt mp" )
                _channels.push_back( pipe->getChannel( ChannelPath( 0 )));
            return TRAVERSE_CONTINUE;
        }

        Window* window = new Window( pipe );
        if( !pipe->getPixelViewport().isValid( ))
            window->setPixelViewport( _pvp );
        window->setIAttribute( WindowSettings::IATTR_HINT_DRAWABLE,
                               fabric::FBO );
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
    LBASSERT( node );
    if( !node )
        return Channels();

    const Pipes& pipes = node->getPipes();
    LBASSERT( !pipes.empty( ));
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

#if 0 // LB_GCC_4_5_OR_LATER
#  pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
void Resources::configure( const Compounds& compounds, const Channels& channels,
                           const fabric::ConfigParams& params )
{
    LBASSERT( !compounds.empty( ));
    if( compounds.empty() || channels.empty( )) // No additional resources
        return;

#ifndef NDEBUG
    const Canvas* canvas = 0;
#endif
    for( CompoundsCIter i = compounds.begin(); i != compounds.end(); ++i )
    {
        const Compounds& children = (*i)->getChildren();
        LBASSERT( children.size() == 1 );
        if( children.size() != 1 )
            continue;

        Compound* segmentCompound = children.front();
#ifndef NDEBUG
        const Channel* channel = segmentCompound->getChannel();
        LBASSERT( channel );
        LBASSERT( !canvas || channel->getCanvas() == canvas );
        canvas = channel->getCanvas();
#endif

        _addMonoCompound( segmentCompound, channels, params );
        _addStereoCompound( segmentCompound, channels, params );
    }
}

static Channels _filter( const Channels& input, const std::string& filter )
{
    Channels result;

    for( ChannelsCIter i = input.begin(); i != input.end(); ++i )
        if( (*i)->getName().find( filter ) != std::string::npos )
            result.push_back( *i );
    return result;
}

Compound* Resources::_addMonoCompound( Compound* root, const Channels& channels,
                                       const fabric::ConfigParams& params )
{
    const Channel* channel = root->getChannel();
    const Layout* layout = channel->getLayout();
    const std::string& name = layout->getName();

    Compound* compound = 0;
    const bool multiProcess =
                    params.getFlags() & fabric::ConfigParams::FLAG_MULTIPROCESS;
    const bool multiProcessDB = multiProcess ||
             ( params.getFlags() & fabric::ConfigParams::FLAG_MULTIPROCESS_DB );
    const Channels& activeChannels = _filter( channels,
                                              multiProcess ? " mp " : " mt " );
    const Channels& activeDBChannels = _filter( channels,
                                                multiProcessDB ? " mp ":" mt ");

    if( name == EQ_SERVER_CONFIG_LAYOUT_SIMPLE )
        /* nop */;
    else if( name == EQ_SERVER_CONFIG_LAYOUT_2D_DYNAMIC ||
             name == EQ_SERVER_CONFIG_LAYOUT_2D_STATIC )
    {
        compound = _add2DCompound( root, activeChannels, params );
    }
    else if( name == EQ_SERVER_CONFIG_LAYOUT_DB_DYNAMIC ||
             name == EQ_SERVER_CONFIG_LAYOUT_DB_STATIC )
    {
        compound = _addDBCompound( root, activeDBChannels, params );
    }
    else if( name == EQ_SERVER_CONFIG_LAYOUT_DB_DS )
        compound = _addDSCompound( root, activeDBChannels );
    else if( name == EQ_SERVER_CONFIG_LAYOUT_DB_2D )
    {
        LBASSERT( !multiProcess );
        LBASSERT( multiProcessDB );
        compound = _addDB2DCompound( root, channels, params );
    }
    else if( name == EQ_SERVER_CONFIG_LAYOUT_SUBPIXEL )
        compound = _addSubpixelCompound( root, activeChannels );
    else
    {
        LBASSERTINFO( false, "Unimplemented mode " << name );
    }

    if( compound )
        compound->setEyes( EYE_CYCLOP );

    return compound;
}

Compound* Resources::_addStereoCompound( Compound* root,
                                         const Channels& channels,
                                         const fabric::ConfigParams& params )
{
    const Channel* channel = root->getChannel();
    const Layout* layout = channel->getLayout();
    const std::string& name = layout->getName();
    if( name == EQ_SERVER_CONFIG_LAYOUT_SIMPLE )
        return 0;

    Compound* compound = new Compound( root );
    compound->setName( "Stereo" );
    compound->setEyes( EYE_LEFT | EYE_RIGHT );

    const bool multiProcess = params.getFlags() &
                                  (fabric::ConfigParams::FLAG_MULTIPROCESS |
                                   fabric::ConfigParams::FLAG_MULTIPROCESS_DB );
    const Channels& active = name == EQ_SERVER_CONFIG_LAYOUT_DB_2D ? channels :
                            _filter( channels, multiProcess ? " mp " : " mt " );

    const size_t nChannels = active.size();
    const ChannelsCIter split = active.begin() + (nChannels >> 1);

    Channels leftChannels( split - active.begin( ));
    std::copy( active.begin(), split, leftChannels.begin( ));

    Channels rightChannels( active.end() - split );
    std::copy( split, active.end(), rightChannels.begin( ));

    Compound* left = 0;
    if( leftChannels.empty() ||
        ( leftChannels.size() == 1 && leftChannels.front() == channel ))
    {
        left = new Compound( compound );
    }
    else
        left = _addMonoCompound( compound, leftChannels, params );

    left->setEyes( EYE_LEFT );

    Compound* right = 0;
    if( rightChannels.empty() ||
        ( rightChannels.size() == 1 && rightChannels.front() == channel ))
    {
        right = new Compound( compound );
    }
    else
        right = _addMonoCompound( compound, rightChannels, params );

    right->setEyes( EYE_RIGHT );

    return compound;
}

Compound* Resources::_add2DCompound( Compound* root, const Channels& channels,
                                     fabric::ConfigParams params )
{
    const Channel* channel = root->getChannel();
    const Layout* layout = channel->getLayout();
    const std::string& name = layout->getName();

    Compound* compound = new Compound( root );
    compound->setName( name );
    if( params.getEqualizer().getMode() == LoadEqualizer::MODE_DB )
        params.getEqualizer().setMode( LoadEqualizer::MODE_2D );

    LoadEqualizer* lb = new LoadEqualizer( params.getEqualizer( ));
    if( name == EQ_SERVER_CONFIG_LAYOUT_2D_STATIC )
        lb->setDamping( 1.f );
    compound->addEqualizer( lb );

    _fill2DCompound( compound, channels );
    return compound;
}

void Resources::_fill2DCompound( Compound* compound, const Channels& channels )
{
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
}

Compound* Resources::_addDBCompound( Compound* root, const Channels& channels,
                                     fabric::ConfigParams params )
{
    const Channel* channel = root->getChannel();
    const Layout* layout = channel->getLayout();
    const std::string& name = layout->getName();

    Compound* compound = new Compound( root );
    compound->setName( name );
    compound->setBuffers( Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH );
    if( name == EQ_SERVER_CONFIG_LAYOUT_DB_DYNAMIC )
    {
        if( params.getEqualizer().getMode() != LoadEqualizer::MODE_DB )
            params.getEqualizer().setMode( LoadEqualizer::MODE_DB );
        compound->addEqualizer( new LoadEqualizer( params.getEqualizer( )));
    }

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
            drawChild->setRange( Range( float( start )/100000.f, 1.f ));
        else
            drawChild->setRange( Range( float( start )/100000.f,
                                        float( start + step )/100000.f ));

        size_t y = 0;
        for( CompoundsCIter j = children.begin(); j != children.end(); ++j )
        {
            if( i != j )
            {
                std::ostringstream frameName;
                frameName << "tile" << j - children.begin() << ".channel"
                          << i - children.begin();
                Viewport vp;
                if(  j+1 == children.end( ) ) // last - correct rounding 'error'
                {
                    vp = Viewport( 0.f, static_cast< float >( y )/100000.f,
                              1.f, static_cast< float >( 100000-y )/100000.f );
                }
                else
                    vp = Viewport( 0.f, static_cast< float >( y )/100000.f,
                                  1.f, static_cast< float >( step )/100000.f );

                eq::server::Frame* outputFrame = new eq::server::Frame;
                outputFrame->setName( frameName.str( ));
                outputFrame->setViewport( vp );
                outputFrame->setBuffers( Frame::BUFFER_COLOR |
                                         Frame::BUFFER_DEPTH );
                drawChild->addOutputFrame( outputFrame );

                // input tiles from other channels
                frameName.str("");
                frameName << "tile" << i - children.begin() << ".channel"
                          << j - children.begin();

                eq::server::Frame* inputFrame = new eq::server::Frame;
                inputFrame->setName( frameName.str( ));
                child->addInputFrame( inputFrame );
            }
            // else own tile, is in place

            y += step;
        }

        // assembled color tile output, if not already in place
        if( child->getChannel() != compound->getChannel( ))
        {
            Frame* output = child->getOutputFrames().front();

            Viewport vp;
            if( i+1 == children.end( )) // last - correct rounding 'error'
            {
                vp = Viewport( 0.f, static_cast< float >( start )/100000.f,
                               1.f,
                               static_cast< float >( 100000-start )/100000.f );
            }
            else
                vp = Viewport( 0.f, static_cast< float >( start )/100000.f,
                               1.f, static_cast< float >( step )/100000.f );

            output->setViewport( vp );
        }
        start += step;
    }

    return compound;
}

static Channels _filterLocalChannels( const Channels& input,
                                      const Compound& filter )
{
    Channels result;
    for( ChannelsCIter i = input.begin(); i != input.end(); ++i )
    {
        const Node* node = (*i)->getNode();
        const Node* filterNode = filter.getChannel()->getNode();
        if( node == filterNode )
            result.push_back( *i );
    }
    return result;
}

Compound* Resources::_addDB2DCompound( Compound* root, const Channels& channels,
                                       fabric::ConfigParams params )
{
    // TODO: Optimized compositing?
    root->setBuffers( Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH );
    const Channels& dbChannels = _filter( channels, " mt mp " );
    Compound* compound = _addDSCompound( root, dbChannels );

    const Compounds& children = compound->getChildren();
    for( CompoundsCIter i = children.begin(); i != children.end(); ++i )
    {
        Compound* child = *i;
        Compound* drawChild = child->getChildren().front();
        if( params.getEqualizer().getMode() == LoadEqualizer::MODE_DB )
            params.getEqualizer().setMode( LoadEqualizer::MODE_2D );
        drawChild->addEqualizer( new LoadEqualizer( params.getEqualizer( )));
        drawChild->setName( EQ_SERVER_CONFIG_LAYOUT_2D_DYNAMIC );

        const Channels& localChannels = _filterLocalChannels( channels,
                                                              *child );
        _fill2DCompound( drawChild, localChannels );
    }

    return compound;
}

Compound* Resources::_addSubpixelCompound( Compound* root,
                                           const Channels& channels )
{
    Compound* compound = new Compound( root );
    compound->setName( EQ_SERVER_CONFIG_LAYOUT_SUBPIXEL );

    const Compounds& children = _addSources( compound, channels, true );
    const uint32_t nChildren = uint32_t( children.size( ));
    for( CompoundsCIter i = children.begin(); i != children.end(); ++i )
        (*i)->setSubPixel( SubPixel( i - children.begin(), nChildren ));

    return compound;
}

const Compounds& Resources::_addSources( Compound* compound,
                                         const Channels& channels,
                                         const bool destChannelFrame )
{
    const Channel* rootChannel = compound->getChannel();
    const Segment* segment = rootChannel->getSegment();
    const Channel* outputChannel = segment ? segment->getChannel() : 0;

    for( ChannelsCIter i = channels.begin(); i != channels.end(); ++i )
    {
        Channel* channel = *i;
        Compound* child = new Compound( compound );

        const bool isDestChannel = channel == outputChannel;
        if( isDestChannel && !destChannelFrame )
            continue;
        if( !isDestChannel )
            child->setChannel( channel );

        Frame* outFrame = new Frame;
        std::stringstream frameName;
        frameName << "Frame." << compound->getName() << '.' << ++_frameCounter;
        outFrame->setName( frameName.str( ));
        if( isDestChannel )
            outFrame->setType( Frame::TYPE_TEXTURE ); // OPT
        child->addOutputFrame( outFrame );

        Frame* inFrame = new Frame;
        inFrame->setName( frameName.str( ));
        compound->addInputFrame( inFrame );
    }

    return compound->getChildren();
}

}
}
}
