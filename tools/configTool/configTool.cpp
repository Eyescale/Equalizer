
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include "configTool.h"
#include "frame.h"

#include <eq/server/equalizers/framerateEqualizer.h>
#include <eq/server/equalizers/loadEqualizer.h>
#include <eq/server/canvas.h>
#include <eq/server/global.h>
#include <eq/server/layout.h>
#include <eq/server/loader.h>
#include <eq/server/node.h>
#include <eq/server/observer.h>
#include <eq/server/segment.h>
#include <eq/server/view.h>
#include <eq/server/window.h>

#include <eq/client/init.h>
#include <eq/client/nodeFactory.h>

#ifndef WIN32
#  include <sys/param.h>
#endif

#include <math.h>

#ifndef MIN
#  define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#include <tclap/CmdLine.h>

#include <iostream>
#include <fstream>


using namespace eq::server;

int main( int argc, char** argv )
{
    ConfigTool configTool;

    if( !configTool.parseArguments( argc, argv ))
        ::exit( EXIT_FAILURE );

    lunchbox::Log::setOutput( std::cout );
    eq::NodeFactory nodeFactory;
    if( !eq::init( 0, 0, &nodeFactory ))
    {
        LBERROR << "Equalizer init failed" << std::endl;
        return EXIT_FAILURE;
    }
    configTool.writeConfig();
}

ConfigTool::ConfigTool()
        :_mode( MODE_2D )
        ,_nPipes( 1 )
        ,_nChannels( 4 )
        ,_useDestination( true )
        ,_columns( 3 )
        ,_rows( 2 )
        ,_nodesFile( "" )
        ,_resX( 960 )
        ,_resY( 600 )
{}

bool ConfigTool::parseArguments( int argc, char** argv )
{
    try
    {
        TCLAP::CmdLine command( "configTool - Equalizer config file generator");

        TCLAP::SwitchArg fullScreenArg( "f", "fullScreen",
                                        "Full screen rendering", command, 
                                        false );

        TCLAP::ValueArg<std::string> modeArg( "m", "mode",
                                         "Compound mode (default 2D)",
                                         false, "2D",
                                         "2D|DB|DB_ds|DB_stream|DPlex|Wall",
                                         command );

        TCLAP::ValueArg<unsigned> pipeArg( "p", "numPipes",
                                         "Number of pipes per node (default 1)",
                                           false, 1, "unsigned", command );

        TCLAP::ValueArg<unsigned> channelArg( "c", "numChannels", 
                                         "Total number of channels (default 4)",
                                              false, 4, "unsigned", command );

        TCLAP::ValueArg<unsigned> columnsArg( "C", "columns", 
                                          "number of columns in a display wall",
                                              false, 3, "unsigned", command );

        TCLAP::ValueArg<unsigned> rowsArg( "R", "rows",
                                           "number of rows in a display wall",
                                           false, 2, "unsigned", command );

        TCLAP::SwitchArg destArg( "a", "assembleOnly", 
                        "Destination channel does not contribute to rendering", 
                                  command, false );

        TCLAP::ValueArg<std::string> nodesArg( "n", "nodes", 
                                          "file with list of node-names",
                                          false, "", "filename", command );

        TCLAP::MultiArg<unsigned> resArg( "r", "resolution",
                                          "output window resolution", 
                                          false, "unsigned", command );

        TCLAP::ValueArg<std::string> descrArg( "d", "descr",
                                      "file with channels per-node description",
                                          false, "", "filename", command );


        command.parse( argc, argv );

        if( nodesArg.isSet( ))
        {
            _nodesFile = nodesArg.getValue();
        }

        if( resArg.isSet( ))
        {
            _resX = resArg.getValue()[0];

            if( resArg.getValue().size() > 1 )
                _resY = resArg.getValue()[1];
        }

        if( pipeArg.isSet( ))
            _nPipes = pipeArg.getValue();
        if( channelArg.isSet( ))
            _nChannels = channelArg.getValue();

        if( columnsArg.isSet( ))
            _columns = columnsArg.getValue();
        if( rowsArg.isSet( ))
            _rows = rowsArg.getValue();

        _useDestination = !destArg.isSet();
        _fullScreen     = fullScreenArg.isSet();

        if( modeArg.isSet( ))
        {
            const std::string& mode = modeArg.getValue();
            if( mode == "2D" )
                _mode = MODE_2D;
            else if( mode == "DB" )
                _mode = MODE_DB;
            else if( mode == "DB_ds" )
                _mode = MODE_DB_DS;
            else if( mode == "DB_stream" )
                _mode = MODE_DB_STREAM;
            else if( mode == "DPlex" )
                _mode = MODE_DPLEX;
            else if( mode == "Wall" )
            {
                _mode   = MODE_WALL;
                _nChannels = _columns * _rows;
            }
            else
            {
                std::cerr << "Unknown mode " << mode << std::endl;
                return false;
            }
        }

        if( descrArg.isSet( ))
        {
            _descrFile = descrArg.getValue();
        }
    }
    catch( TCLAP::ArgException& exception )
    {
        std::cerr << "Command line parse error: " << exception.error() 
             << " for argument " << exception.argId() << std::endl;
        return false;
    }
    return true;
}

static Strings readNodenames( const std::string &filename )
{
    Strings nodeNames;
    if( filename.empty( ))
        return nodeNames;

    std::ifstream inStream;
    inStream.open( filename.c_str() );
    if( !inStream.is_open() )
      return nodeNames;

    std::string name;
    while( inStream >> name )
        nodeNames.push_back( name );

    inStream.close();
    return nodeNames;
}

void ConfigTool::writeConfig() const
{
    Global* global = Global::instance();
    global->setConfigFAttribute( Config::FATTR_VERSION, 1.2f );
    global->setWindowIAttribute( eq::server::Window::IATTR_HINT_FULLSCREEN,
                                 eq::fabric::ON );
    if( _mode != MODE_WALL )
    {
        global->setWindowIAttribute(eq::server::Window::IATTR_HINT_DOUBLEBUFFER,
                                    eq::fabric::OFF );
        global->setWindowIAttribute( eq::server::Window::IATTR_HINT_DRAWABLE,
                                     eq::fabric::PBUFFER );
    }
    if( _mode >= MODE_DB && _mode <= MODE_DB_STREAM )
        global->setWindowIAttribute( eq::server::Window::IATTR_PLANES_STENCIL,
                                     eq::fabric::ON );

    ServerPtr server = new eq::server::Server;
    const Strings nodeNames = readNodenames( _nodesFile );
    co::ConnectionDescriptionPtr desc = new ConnectionDescription;
    if( !nodeNames.empty( ))
        desc->setHostname( nodeNames.front( ));
    server->addConnectionDescription( desc );

    Config* config = new Config( server );
    _writeResources( config, nodeNames );
    _writeCompound( config );

    lunchbox::Log::instance( "", 0 )
        << lunchbox::disableHeader << global << *server << std::endl
        << lunchbox::enableHeader << lunchbox::disableFlush;
}

void ConfigTool::_writeResources( Config* config,
                                  const Strings& nodeNames ) const
{
    const unsigned nNodes  = _nChannels/_nPipes + 1;
    unsigned       c = 0;

    for( unsigned n=0; n < nNodes && c < _nChannels; ++n )
    {
        Node* node = new Node( config );
        if( n == 0 )
            node->setApplicationNode( true );

        std::ostringstream nodeName;
        if( n < nodeNames.size() )
            nodeName << nodeNames[ n ];
        else
            nodeName << "node" << n;

        node->setHost( nodeName.str( ));
        node->setName( nodeName.str( ));

        co::ConnectionDescriptionPtr connectionDescription = 
            new co::ConnectionDescription;
        connectionDescription->setHostname( nodeName.str( ));
        node->addConnectionDescription( connectionDescription );

        for( unsigned p=0; p < _nPipes && c < _nChannels; ++p )
        {
            Pipe* pipe = new Pipe( node );

            std::ostringstream pipeName;
            pipeName << "pipe" << p << "n" << n;
            pipe->setName( pipeName.str( ));
            pipe->setDevice( p );
            
            eq::server::Window* window = new eq::server::Window( pipe );

            std::ostringstream windowName;
            windowName << "window" << c;
            window->setName( windowName.str( ));

            if( c == 0 ) // destination window
            {
                if( !_fullScreen && _mode != MODE_WALL )
                    window->setIAttribute( 
                        eq::server::Window::IATTR_HINT_FULLSCREEN,
                        eq::fabric::OFF );
                window->setIAttribute( eq::server::Window::IATTR_HINT_DRAWABLE, 
                                       eq::fabric::WINDOW );
                window->setIAttribute(
                    eq::server::Window::IATTR_HINT_DOUBLEBUFFER, 
                    eq::fabric::ON );
            }

            if( _mode == MODE_WALL || c == 0 )
            {
                eq::PixelViewport viewport( 0, 0, _resX, _resY );
                if( !_fullScreen && c == 0 )
                {
                    viewport.x = _resX >> 1;
                    viewport.y = 100;
                }
                window->setPixelViewport( viewport );
            }

            Channel* channel = new Channel( window );
            std::ostringstream channelName;
            channelName << "channel" << c;
            channel->setName( channelName.str( ));

            ++c;            
        }
    }
}

void ConfigTool::_writeCompound( Config* config ) const
{
  if( !_descrFile.empty( ))
    {
        _writeFromDescription( config );
        return;
    }

    switch( _mode )
    {
        case MODE_2D:
            _write2D( config );
            break;

        case MODE_DB:
            _writeDB( config );
            break;

        case MODE_DB_DS:
            _writeDS( config );
            break;

        case MODE_DB_STREAM:
            _writeDBStream( config );
            break;

        case MODE_DPLEX:
            _writeDPlex( config );
            break;

        case MODE_WALL:
            _writeWall( config );
            break;

        default:
            std::cerr << "Unimplemented mode " << unsigned(_mode) << std::endl;
            exit( EXIT_FAILURE );
    }
}

eq::server::Compound* ConfigTool::_addSingleSegment( Config* config ) const
{
    Channel* channel = config->find< Channel >( "channel0" );
    Observer* observer = new Observer( config );

    Wall wall;
    wall.resizeHorizontalToAR( float( _resX ) / float( _resY ));

    Canvas* canvas = new Canvas( config );
    canvas->setWall( wall );
    Segment* segment = new Segment( canvas );
    segment->setChannel( channel );

    Layout* layout = new Layout( config );
    View* view = new View( layout );
    view->setObserver( observer );
    view->setWall( wall );

    canvas->addLayout( layout );
    config->activateCanvas( canvas );

    const Compounds compounds = Loader::addOutputCompounds(config->getServer());
    LBASSERT( compounds.size() == 1 );
    if( compounds.empty( ))
        return 0;

    Compound* root = compounds.front();
    const Compounds& children = root->getChildren();
    LBASSERT( children.size() == 1 );
    return children.empty() ? 0 : children.front();
}

void ConfigTool::_write2D( Config* config ) const
{
    Compound* compound = _addSingleSegment( config );
    compound->addEqualizer( new LoadEqualizer );

    const unsigned step = static_cast< unsigned >
        ( 100000.0f / ( _useDestination ? _nChannels : _nChannels - 1 ));
    unsigned y = 0;
    for( unsigned i = _useDestination ? 0 : 1; i<_nChannels; ++i )
    {
        Compound* child = new Compound( compound );
        std::ostringstream channelName;
        channelName << "channel" << i;
        
        Channel* childChannel = config->find< Channel >( channelName.str( ));
        child->setChannel( childChannel );

        if( i == _nChannels - 1 ) // last - correct rounding 'error'
        {
            if( y!=0 ) // more than one channel
            {
                child->setViewport( 
                    eq::Viewport( 0.f, static_cast< float >( y )/100000.f,
                                  1.f,
                                  static_cast< float >( 100000-y )/100000.f ));
            }
        }
        else
            child->setViewport( 
                eq::Viewport( 0.f, static_cast< float >( y )/100000.f,
                              1.f, static_cast< float >( step )/100000.f ));

        if( i != 0 )
        {
            std::ostringstream frameName;
            frameName << "frame.channel" << i;

            child->addOutputFrame(   ::Frame::create( frameName ));
            compound->addInputFrame( ::Frame::create( frameName ));
        }
 
        y += step;
    }
}

void ConfigTool::_writeDB( Config* config ) const
{
    Compound* compound = _addSingleSegment( config );
    compound->setBuffers( eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH );

    if( !_useDestination )
        compound->setTasks( eq::fabric::TASK_CLEAR|eq::fabric::TASK_ASSEMBLE );

    const unsigned step = static_cast< unsigned >
        ( 100000.0f / ( _useDestination ? _nChannels : _nChannels - 1 ));
    unsigned start = 0;

    for( unsigned i = _useDestination ? 0 : 1; i<_nChannels; ++i )
    {
        Compound* child = new Compound( compound );
        std::ostringstream channelName;
        channelName << "channel" << i;
        
        Channel* childChannel = config->find< Channel >( channelName.str( ));
        child->setChannel( childChannel );

        if( i == _nChannels - 1 ) // last - correct rounding 'error'
        {
            if( i!=0 ) // more than one channel
            {
                child->setRange(
                    eq::Range( static_cast< float >( start )/100000.f, 1.f ));
            }
        }
        else
            child->setRange(
                eq::Range( static_cast< float >( start )/100000.f,
                           static_cast< float >( start + step )/100000.f ));

        if( i != 0 )
        {
            std::ostringstream frameName;
            frameName << "frame.channel" << i;

            child->addOutputFrame(   ::Frame::create( frameName ));
            compound->addInputFrame( ::Frame::create( frameName ));
        }
 
        start += step;
    }
}

void ConfigTool::_writeDBStream( Config* config ) const
{
    Compound* compound = _addSingleSegment( config );
    compound->setBuffers( eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH );

    if( !_useDestination )
        compound->setTasks( eq::fabric::TASK_CLEAR|eq::fabric::TASK_ASSEMBLE );

    const unsigned nDraw = _useDestination ? _nChannels : _nChannels - 1;
    const unsigned step  = static_cast< unsigned >( 100000.0f / ( nDraw ));
    const unsigned stop  = _useDestination ? 0 : 1;
    unsigned start = 0;
    for( unsigned i = _nChannels; i>stop; --i )
    {
        Compound* child = new Compound( compound );
        std::ostringstream channelName;
        channelName << "channel" << i-1;
        
        Channel* childChannel = config->find< Channel >( channelName.str( ));
        child->setChannel( childChannel );

        if( i-1 == stop ) // last - correct rounding error
            child->setRange(
                eq::Range( static_cast< float >( start )/100000.f, 1.f ));
        else
            child->setRange(
                eq::Range( static_cast< float >( start )/100000.f,
                           static_cast< float >( start + step )/100000.f ));

        if( i != _nChannels )
        {
            std::ostringstream frameName;
            frameName << "frame.channel" << i;

            child->addInputFrame( ::Frame::create( frameName ));
        }

        if( i != 1 ) 
        {
            std::ostringstream frameName;
            frameName << "frame.channel" << i-1;

            child->addOutputFrame( ::Frame::create( frameName ));
        }
 
        start += step;
    }
    if( !_useDestination )
        compound->addInputFrame( ::Frame::create( "frame.channel1" ));
}

void ConfigTool::_writeDS( Config* config ) const
{
    Compound* compound = _addSingleSegment( config );
    compound->setBuffers( eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH );

    const unsigned step = static_cast< unsigned >
        ( 100000.0f / ( _useDestination ? _nChannels : _nChannels - 1 ));
    unsigned start = 0;
    for( unsigned i = _useDestination ? 0 : 1; i<_nChannels; ++i )
    {
        Compound* child = new Compound( compound );
        std::ostringstream channelName;
        channelName << "channel" << i;
        
        Channel* childChannel = config->find< Channel >( channelName.str( ));
        child->setChannel( childChannel );

        // leaf draw + tile readback compound
        Compound* drawChild = new Compound( child );
        if( i == _nChannels - 1 ) // last - correct rounding 'error'
        {
            drawChild->setRange(
                eq::Range( static_cast< float >( start )/100000.f, 1.f ));
        }
        else
            drawChild->setRange(
                eq::Range( static_cast< float >( start )/100000.f,
                           static_cast< float >( start + step )/100000.f ));
        
        unsigned y = 0;
        for( unsigned j = _useDestination ? 0 : 1; j<_nChannels; ++j )
        {
            if( i != j )
            {
                std::ostringstream frameName;
                frameName << "tile" << j << ".channel" << i;

                eq::Viewport vp;
                if( j == _nChannels - 1 ) // last - correct rounding 'error'
                {
                    vp = eq::Viewport( 0.f, static_cast< float >( y )/100000.f,
                              1.f, static_cast< float >( 100000-y )/100000.f );
                }
                else
                    vp = eq::Viewport( 0.f, static_cast< float >( y )/100000.f,
                                  1.f, static_cast< float >( step )/100000.f );

                drawChild->addOutputFrame( ::Frame::create( frameName, vp ));

                // input tiles from other channels
                frameName.str("");
                frameName << "tile" << i << ".channel" << j;

                child->addInputFrame(      ::Frame::create( frameName ));
            }
            // else own tile, is in place

            y += step;
        }
 
        // assembled color tile output, if not already in place
        if( i != 0 )
        {
            std::ostringstream frameName;
            frameName << "frame.channel" << i;

            eq::Viewport vp;
            if( i == _nChannels - 1 ) // last - correct rounding 'error'
            {
                vp = eq::Viewport( 0.f, static_cast< float >( start )/100000.f,
                                  1.f,
                               static_cast< float >( 100000-start )/100000.f );
            }
            else
                vp = eq::Viewport( 0.f, static_cast< float >( start )/100000.f,
                                  1.f, static_cast< float >( step )/100000.f );

            child->addOutputFrame(   ::Frame::create( frameName, vp, true ));
            compound->addInputFrame( ::Frame::create( frameName ));
        }
        start += step;
    }
}

void ConfigTool::_writeDPlex( Config* config ) const
{
    Compound* compound = _addSingleSegment( config );
    compound->setTasks( eq::fabric::TASK_CLEAR | eq::fabric::TASK_ASSEMBLE );
    compound->addEqualizer( new FramerateEqualizer );
    
    const unsigned period = _nChannels - 1;
    unsigned       phase  = 0;
    for( unsigned i = 1; i<_nChannels; ++i )
    {
        Compound* child = new Compound( compound );
        std::ostringstream channelName;
        channelName << "channel" << i;
        
        Channel* childChannel = config->find< Channel >( channelName.str( ));
        child->setChannel( childChannel );

        child->setPeriod( period );
        child->setPhase( phase );
        child->addOutputFrame( ::Frame::create( "frame.DPlex" ));

        ++phase;
    }

    compound->addInputFrame( ::Frame::create( "frame.DPlex" ));
}

void ConfigTool::_writeWall( Config* config ) const
{
    Layout* layout = new Layout( config );
    layout->setName( "1x1" );
    new View( layout );

    Canvas* canvas = new Canvas( config );
    canvas->setName( "wall" );
    canvas->setSwapBarrier( new SwapBarrier );

    eq::Wall wall;
    canvas->setWall( wall );

    canvas->addLayout( layout );
    config->activateCanvas( canvas );

    const float width  = 1.0f / static_cast< float >( _rows );
    const float height = 1.0f / static_cast< float >( _columns );

    for( unsigned row = 0; row < _rows; ++row )
    {
        for( unsigned column = 0; column < _columns; ++column )
        {
            Segment* segment = new Segment( canvas );
            std::ostringstream segmentName;
            segmentName << "segment_" << row << "_" << column;
            segment->setName( segmentName.str( ));
            segment->setViewport( eq::Viewport( width * row, height * column,
                                                width, height ));
            std::ostringstream channelName;
            channelName << "channel" << column + row * _columns;
            Channel* segmentChannel = 
                config->find< Channel >( channelName.str( ));
            segment->setChannel( segmentChannel );
        }
    }
    config->activateCanvas( canvas );    
}
