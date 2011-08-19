
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
#include <eq/server/node.h>
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

    co::base::Log::setOutput( std::cout );
    eq::NodeFactory nodeFactory;
    if( !eq::init( 0, 0, &nodeFactory ))
    {
        EQERROR << "Equalizer init failed" << std::endl;
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
        ,_resX( 1024 )
        ,_resY( 768 )
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
                                    "2D|DB|DB_ds|DB_stream|DB_ds_ac|DPlex|Wall",
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
            else if( mode == "DB_ds_ac" )
            {
                _mode = MODE_DB_DS_AC;
                _nPipes = 2;

                if( _nChannels%2 != 0 )
                {
                    std::cerr << "Channels number must be even" << std::endl;
                    return false;
                }
            }
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

void ConfigTool::writeConfig() const
{
    Global* global = Global::instance();
    global->setWindowIAttribute( eq::server::Window::IATTR_HINT_FULLSCREEN,
                                 eq::fabric::ON );
    global->setWindowIAttribute( eq::server::Window::IATTR_HINT_DOUBLEBUFFER,
                                 eq::fabric::OFF );
    global->setWindowIAttribute( eq::server::Window::IATTR_HINT_DRAWABLE,
                                 eq::fabric::PBUFFER );

    if( _mode >= MODE_DB && _mode <= MODE_DB_DS_AC )
        global->setWindowIAttribute( eq::server::Window::IATTR_PLANES_STENCIL,
                                     eq::fabric::ON );

    ServerPtr server = new eq::server::Server;
    Config* config = new Config( server );

    _writeResources( config );
    _writeCompound( config );

    co::base::Log::instance( "", 0 )
        << co::base::disableHeader << global << *server << std::endl
        << co::base::enableHeader << co::base::disableFlush;
}

static void readNodenames
(
    const std::string &filename,
    const unsigned maxNodes,
    std::vector< std::string > &nodesNames
)
{
    if( filename == "" )
        return;

    std::ifstream inStream;

    inStream.open( filename.c_str() );
    if ( inStream.is_open() )
    {
        std::string tmp;
        unsigned pos = 0;
        while( ++pos < maxNodes && inStream >> tmp )
            nodesNames.push_back( tmp );

        inStream.close();
    }
}

void ConfigTool::_writeResources( Config* config ) const
{
    const unsigned nNodes  = _nChannels/_nPipes + 1;
    unsigned       c = 0;
    std::vector<std::string> nodesNames;

    readNodenames( _nodesFile, nNodes, nodesNames );

    for( unsigned n=0; n < nNodes && c < _nChannels; ++n )
    {
        Node* node = new Node( config );

        std::ostringstream nodeName;
        if( n < nodesNames.size() )
            nodeName << nodesNames[ n ];
        else
        {
            nodeName << "node" << n;
            if( n == 0 )
                node->setApplicationNode( true );
        }

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
                if( !_fullScreen )
                    window->setIAttribute( 
                        eq::server::Window::IATTR_HINT_FULLSCREEN,
                        eq::fabric::OFF );
                window->setIAttribute( eq::server::Window::IATTR_HINT_DRAWABLE, 
                                       eq::fabric::WINDOW );
                window->setIAttribute(
                    eq::server::Window::IATTR_HINT_DOUBLEBUFFER, 
                    eq::fabric::ON );
            }

            eq::PixelViewport viewport( 0, 0, _resX, _resY );
            if( !_fullScreen && c == 0 )
            {
                viewport.x = 100;
                viewport.y = 100;
            }
            window->setPixelViewport( viewport );

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
    if( _descrFile != "" )
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

        case MODE_DB_DS_AC:
            _writeDSAC( config );
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

void ConfigTool::_write2D( Config* config ) const
{
    Compound* compound = new Compound( config );
    Channel* channel = config->find< Channel >( "channel0" );
    compound->setChannel( channel );
    
    eq::Wall wall;
    wall.bottomLeft  = eq::Vector3f( -.32f, -.2f, -.75f );
    wall.bottomRight = eq::Vector3f(  .32f, -.2f, -.75f );
    wall.topLeft     = eq::Vector3f( -.32f,  .2f, -.75f );
    compound->setWall( wall );
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
    Compound* compound = new Compound( config );
    Channel* channel = config->find< Channel >( "channel0" );
    compound->setChannel( channel );
    compound->setBuffers( eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH );

    if( !_useDestination )
        compound->setTasks( eq::fabric::TASK_CLEAR|eq::fabric::TASK_ASSEMBLE );

    eq::Wall wall;
    wall.bottomLeft  = eq::Vector3f( -.32f, -.2f, -.75f );
    wall.bottomRight = eq::Vector3f(  .32f, -.2f, -.75f );
    wall.topLeft     = eq::Vector3f( -.32f,  .2f, -.75f );
    compound->setWall( wall );

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
    Compound* compound = new Compound( config );
    Channel* channel = config->find< Channel >( "channel0" );

    compound->setChannel( channel );
    compound->setBuffers( eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH );

    if( !_useDestination )
        compound->setTasks( eq::fabric::TASK_CLEAR|eq::fabric::TASK_ASSEMBLE );

    eq::Wall wall;
    wall.bottomLeft  = eq::Vector3f( -.32f, -.2f, -.75f );
    wall.bottomRight = eq::Vector3f(  .32f, -.2f, -.75f );
    wall.topLeft     = eq::Vector3f( -.32f,  .2f, -.75f );
    compound->setWall( wall );

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
    Compound* compound = new Compound( config );
    Channel* channel = config->find< Channel >( "channel0" );
    compound->setChannel( channel );
    compound->setBuffers( eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH );

    eq::Wall wall;
    wall.bottomLeft  = eq::Vector3f( -.32f, -.2f, -.75f );
    wall.bottomRight = eq::Vector3f(  .32f, -.2f, -.75f );
    wall.topLeft     = eq::Vector3f( -.32f,  .2f, -.75f );
    compound->setWall( wall );


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

// each node has two GPUs and compositing is performed on second GPU
void ConfigTool::_writeDSAC( Config* config ) const
{
    std::cout << "        compound" << std::endl
         << "        {" << std::endl
         << "            channel   \"channel0\"" << std::endl
         << "            buffer    [ COLOR DEPTH ]" << std::endl
         << "            wall" << std::endl
         << "            {" << std::endl
         << "                bottom_left  [ -.32 -.20 -.75 ]" << std::endl
         << "                bottom_right [  .32 -.20 -.75 ]" << std::endl
         << "                top_left     [ -.32  .20 -.75 ]" << std::endl
         << "            }" << std::endl;

    unsigned nChannels = _nChannels / 2;
    unsigned start      = 0;
    const unsigned step = static_cast< unsigned >( 100000.0f / nChannels );
    for( unsigned i = 0; i<nChannels; ++i, start += step )
    {
        // Rendering compund
        std::cout << "            compound" << std::endl
             << "            {" << std::endl
             << "                channel   \"channel" << i*2+1 << "\""
             << std::endl;

        // leaf draw + tile readback compound
        std::cout << "                compound" << std::endl
             << "                {" << std::endl;
        if( i == nChannels - 1 ) // last - correct rounding 'error'
            std::cout << "                    range     [ 0." << std::setw(5) << start 
                 << " 1 ]" << std::endl;
        else
            std::cout << "                    range     [ 0." << std::setw(5) << start 
                 << " 0." << std::setw(5) << start + step << " ]" << std::endl;
        
        unsigned y = 0;
        for( unsigned j = 0; j<nChannels; ++j )
        {
            std::cout << "                    outputframe{ name \"tile" << j*2+1 
                 << ".channel" << i*2+1 << "\" ";
            if( j == nChannels - 1 ) // last - correct rounding 'error'
                std::cout << "viewport [ 0 0." << std::setw(5) << y << " 1 0." << std::setw(5)
                     << 100000-y << " ]";
            else
                std::cout << "viewport [ 0 0." << std::setw(5) << y << " 1 0." << std::setw(5)
                     << step << " ]";
            std::cout << " }" << std::endl;

            y += step;
        }
        std::cout << "                }" << std::endl
             << "            }" << std::endl
             << std::endl;

        // compositing compund
        std::cout << "            compound" << std::endl
             << "            {" << std::endl
             << "                channel   \"channel" << i*2 << "\""
             << std::endl;

        // input tiles from other channels
        for( unsigned j = 0; j<nChannels; ++j )
        {
            std::cout << "                inputframe{ name \"tile" << i*2+1
                 << ".channel" << j*2+1 <<"\" }" << std::endl;
        }

        // assembled color tile output, if not already in place
        if( i!=0 )
        {
            std::cout << "                outputframe{ name \"frame.channel" 
                 << i*2 <<"\"";
            std::cout << " buffer [ COLOR ] ";
            if( i == nChannels - 1 ) // last - correct rounding 'error'
                std::cout << "viewport [ 0 0." << std::setw(5) << start << " 1 0."
                     << std::setw(5) << 100000 - start << " ]";
            else
                std::cout << "viewport [ 0 0." << std::setw(5) << start << " 1 0."
                     << std::setw(5) << step << " ]";
            std::cout << " }" << std::endl;
        }


        std::cout << "            }" << std::endl
             << std::endl;
    }

    // gather assembled input tiles
    for( unsigned i = 1; i<nChannels; ++i )
        std::cout << "            inputframe{ name \"frame.channel" << i*2 << "\" }" 
             << std::endl;
    std::cout << "        }" << std::endl;    
}

void ConfigTool::_writeDPlex( Config* config ) const
{
    Compound* compound = new Compound( config );
    Channel* channel = config->find< Channel >( "channel0" );
    compound->setChannel( channel );
    
    eq::Wall wall;
    wall.bottomLeft  = eq::Vector3f( -.32f, -.2f, -.75f );
    wall.bottomRight = eq::Vector3f(  .32f, -.2f, -.75f );
    wall.topLeft     = eq::Vector3f( -.32f,  .2f, -.75f );
    compound->setWall( wall );
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
    
    eq::Wall wall;
    wall.bottomLeft  = eq::Vector3f( -.32f, -.2f, -.75f );
    wall.bottomRight = eq::Vector3f(  .32f, -.2f, -.75f );
    wall.topLeft     = eq::Vector3f( -.32f,  .2f, -.75f );
    canvas->setWall( wall );

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
