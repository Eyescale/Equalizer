
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "configTool.h"


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


using namespace std;

int main( int argc, char** argv )
{
    ConfigTool configTool;

    if( configTool.parseArguments( argc, argv ))
        configTool.writeConfig();
}

ConfigTool::ConfigTool()
        : _mode( MODE_2D ),
          _nPipes( 1 ),
          _nChannels( 4 ),
          _useDestination( true ),
          _wallW( 3 ),
          _wallH( 2 ),
          _nodesFile( "" )
{}

bool ConfigTool::parseArguments( int argc, char** argv )
{
    try
    {
        TCLAP::CmdLine command( "configTool - Equalizer config file generator" );

        TCLAP::SwitchArg fullScreenArg( "f", "fullScreen",
                            "Full screen rendering", command, false );

        TCLAP::ValueArg<string> modeArg( "m", "mode", "Compound mode (default 2D)",
                                         false, "2D", "2D|DB|DB_ds|DB_ds_ac|Wall", command );

        TCLAP::ValueArg<unsigned> pipeArg( "p", "numPipes",
                                           "Number of pipes per node (default 1)",
                                           false, 1, "unsigned", command );

        TCLAP::ValueArg<unsigned> channelArg( "c", "numChannels", 
                                             "Total number of channels (default 4)",
                                              false, 4, "unsigned", command );

        TCLAP::ValueArg<unsigned> wallWArg( "W", "wallWidth", 
                                             "number of displays in a raw in a display wall",
                                              false, 3, "unsigned", command );

        TCLAP::ValueArg<unsigned> wallHArg( "H", "wallHeight",
                                             "number of displays' raws in a display wall",
                                              false, 2, "unsigned", command );

        TCLAP::SwitchArg destArg( "a", "assembleOnly", 
                            "Destination channel does not contribute to rendering", 
                                  command, false );

        TCLAP::ValueArg<string> nodesArg( "n", "nodes", "file with list of node-names",
                                         false, "", "", command );

        command.parse( argc, argv );

        if( nodesArg.isSet( ))
        {
            _nodesFile = nodesArg.getValue();
        }


        if( pipeArg.isSet( ))
            _nPipes = pipeArg.getValue();
        if( channelArg.isSet( ))
            _nChannels = channelArg.getValue();

        if( wallWArg.isSet( ))
            _wallW = wallWArg.getValue();
        if( wallHArg.isSet( ))
            _wallH = wallHArg.getValue();

        _useDestination = !destArg.isSet();
        _fullScreen     = fullScreenArg.isSet();

        if( modeArg.isSet( ))
        {
            const string& mode = modeArg.getValue();
            if( mode == "2D" )
                _mode = MODE_2D;
            else if( mode == "DB" )
                _mode = MODE_DB;
            else if( mode == "DB_ds" )
                _mode = MODE_DB_DS;
            else if( mode == "DB_ds_ac" )
            {
                _mode = MODE_DB_DS_AC;
                _nPipes = 2;

                if( _nChannels%2 != 0 )
                {
                    cerr << "Channels number must be even" << endl;
                    return false;
                }
            }
            else if( mode == "Wall" )
            {
                _mode   = MODE_WALL;
                _nChannels = _wallW * _wallH;
            }
            else
            {
                cerr << "Unknown mode " << mode << endl;
                return false;
            }
        }
    }
    catch( TCLAP::ArgException& exception )
    {
        cerr << "Command line parse error: " << exception.error() 
             << " for argument " << exception.argId() << endl;
        return false;
    }
    return true;
}

void ConfigTool::writeConfig() const
{
    cout.setf( std::ios::right, std::ios::adjustfield );
    cout.fill( '0' );

    cout << "global" << endl;
    cout << "{" << endl;
    cout << "    EQ_WINDOW_IATTR_HINT_FULLSCREEN    ON" << endl;
    if( _mode != MODE_2D )
        cout << "    EQ_WINDOW_IATTR_PLANES_STENCIL     ON" << endl;
    cout << "}" << endl;

    cout << "server" << endl;
    cout << "{" << endl;
    cout << "    config" << endl;
    cout << "    {" << endl;

    _writeResources();
    _writeCompound();

    cout << "    }" << endl;
    cout << "}" << endl;
}

void readNodenames
(
    const string &filename,
    const unsigned maxNodes,
    vector<string> &nodesNames
)
{
    if( filename == "" )
        return;

    std::ifstream inStream;

    inStream.open( filename.c_str() );
    if ( inStream.is_open() )
    {
        string tmp;
        unsigned pos = 0;
        while( ++pos < maxNodes && inStream >> tmp )
            nodesNames.push_back( tmp );

        inStream.close();
    }
}

void ConfigTool::_writeResources() const
{

    const unsigned nNodes  = _nChannels/_nPipes + 1;
    unsigned       channel = 0;

    vector<string> nodesNames;

    readNodenames( _nodesFile, nNodes, nodesNames );

    for( unsigned node=0; node < nNodes && channel < _nChannels; ++node )
    {
        std::ostringstream nodeName;
        if( node < nodesNames.size() )
            nodeName << nodesNames[node];
        else
            nodeName << "node" << node;

        cout << "        node" << endl
             << "        {" << endl
             << "            name       \"node" << node << "\""<< endl
             << "            connection { hostname \"" << nodeName.str() << "\" }"<< endl;
        for( unsigned pipe=0; pipe < _nPipes && channel < _nChannels; ++pipe )
        {
            cout << "            pipe" << endl
                 << "            {" << endl
                 << "                name     \"pipe" << pipe << "n" << node << "\"" 
                 << endl
                 << "                device   " << pipe << endl
                 << "                window" << endl
                 << "                {" << endl
                 << "                    name     \"window" << channel << "\""
                 << endl;
            if( channel == 0 && !_fullScreen ) // destination window
                cout << "                    attributes{ hint_fullscreen OFF }"
                     << endl
                     << "                    viewport [ 100 100 1280 800 ]" << endl;

            cout << "                    channel" << endl
                 << "                    {" << endl
                 << "                        name     \"channel" << channel 
                 << "\"" << endl
                 << "                    }" << endl
                 << "                }" << endl
                 << "            }" << endl;
            ++channel;
        }
        cout << "        }" << endl;
    }
}

void ConfigTool::_writeCompound() const
{
    switch( _mode )
    {
        case MODE_2D:
            _write2D();
            break;

        case MODE_DB:
            _writeDB();
            break;

        case MODE_DB_DS:
            _writeDS();
            break;

        case MODE_DB_DS_AC:
            _writeDSAC();
            break;

        case MODE_WALL:
            _writeWall();
            break;

        default:
            cerr << "Unimplemented mode " << static_cast< unsigned >( _mode )
                 << endl;
            exit( EXIT_FAILURE );
    }
}

void ConfigTool::_write2D() const
{
    cout << "        compound" << endl
         << "        {" << endl
         << "            channel   \"channel0\"" << endl
         << "            wall" << endl
         << "            {" << endl
         << "                bottom_left  [ -.32 -.20 -.75 ]" << endl
         << "                bottom_right [  .32 -.20 -.75 ]" << endl
         << "                top_left     [ -.32  .20 -.75 ]" << endl
         << "            }" << endl;

    const unsigned step = static_cast< unsigned >
        ( 100000.0f / ( _useDestination ? _nChannels : _nChannels - 1 ));
    unsigned y = 0;
    for( unsigned i = _useDestination ? 0 : 1; i<_nChannels; ++i )
    {
        cout << "            compound" << endl
             << "            {" << endl
             << "                channel   \"channel" << i << "\"" << endl;

        if( i == _nChannels - 1 ) // last - correct rounding 'error'
        {
            if( y==0 ) //only one channel
            {
                cout << "                viewport  [ 0 0 1 1 ]" << endl;
            }else
            {
                cout << "                viewport  [ 0 0."
                     << setw(5) << y << " 1 0."
                     << setw(5) << 100000-y << " ]" << endl;
            }
        }
        else
            cout << "                viewport  [ 0 0." << setw(5) << y << " 1 0."
                 << setw(5) << step << " ]" << endl;

        if( i != 0 ) 
            cout << "                outputframe{ name \"frame.channel" << i <<"\"}"
                 << endl;

        cout << "            }" << endl;

        if( i != 0 ) 
	  cout << "            inputframe{ name \"frame.channel" << i <<"\" }" 
	       << endl << endl;
 
        y += step;
    }

    cout << "        }" << endl;    
}

void ConfigTool::_writeDB() const
{
    cout << "        compound" << endl
         << "        {" << endl
         << "            channel   \"channel0\"" << endl
         << "            buffer    [ COLOR DEPTH ]" << endl
         << "            wall" << endl
         << "            {" << endl
         << "                bottom_left  [ -.32 -.20 -.75 ]" << endl
         << "                bottom_right [  .32 -.20 -.75 ]" << endl
         << "                top_left     [ -.32  .20 -.75 ]" << endl
         << "            }" << endl;
    if( !_useDestination )
        cout << "            task      [ CLEAR ASSEMBLE ]" << endl;

    const unsigned step = static_cast< unsigned >
        ( 100000.0f / ( _useDestination ? _nChannels : _nChannels - 1 ));
    unsigned start = 0;
    for( unsigned i = _useDestination ? 0 : 1; i<_nChannels; ++i )
    {
        cout << "            compound" << endl
             << "            {" << endl
             << "                channel   \"channel" << i << "\"" << endl;

        if( i == _nChannels - 1 ) // last - correct rounding 'error'
            cout << "                range     [ 0." << setw(5) << start << " 1 ]"
                 << endl;
        else
            cout << "                range     [ 0." << setw(5) << start << " 0."
                 << setw(5) << start + step << " ]" << endl;

        if( i != 0 ) 
            cout << "                outputframe{ name \"frame.channel" << i <<"\"}"
                 << endl;

        cout << "            }" << endl;

	if( i != 0 )
	  cout << "            inputframe{ name \"frame.channel" << i <<"\" }" << endl
	       << endl;
 
        start += step;
    }

    cout << "        }" << endl;    
}

void ConfigTool::_writeDS() const
{
    cout << "        compound" << endl
         << "        {" << endl
         << "            channel   \"channel0\"" << endl
         << "            buffer    [ COLOR DEPTH ]" << endl
         << "            wall" << endl
         << "            {" << endl
         << "                bottom_left  [ -.32 -.20 -.75 ]" << endl
         << "                bottom_right [  .32 -.20 -.75 ]" << endl
         << "                top_left     [ -.32  .20 -.75 ]" << endl
         << "            }" << endl;

    const unsigned step = static_cast< unsigned >
        ( 100000.0f / ( _useDestination ? _nChannels : _nChannels - 1 ));
    unsigned start = 0;
    for( unsigned i = _useDestination ? 0 : 1; i<_nChannels; ++i )
    {
        cout << "            compound" << endl
             << "            {" << endl
             << "                channel   \"channel" << i << "\"" << endl;

        // leaf draw + tile readback compound
        cout << "                compound" << endl
             << "                {" << endl;
        if( i == _nChannels - 1 ) // last - correct rounding 'error'
            cout << "                    range     [ 0." << setw(5) << start 
                 << " 1 ]" << endl;
        else
            cout << "                    range     [ 0." << setw(5) << start 
                 << " 0." << setw(5) << start + step << " ]" << endl;
        
        unsigned y = 0;
        for( unsigned j = _useDestination ? 0 : 1; j<_nChannels; ++j )
        {
            if( i != j )
            {
                cout << "                    outputframe{ name \"tile" << j 
                     << ".channel" << i << "\" ";
                if( j == _nChannels - 1 ) // last - correct rounding 'error'
                    cout << "viewport [ 0 0." << setw(5) << y << " 1 0." << setw(5)
                         << 100000-y << " ]";
                else
                    cout << "viewport [ 0 0." << setw(5) << y << " 1 0." << setw(5)
                         << step << " ]";
                cout << " }" << endl;
            }
            // else own tile, is in place

            y += step;
        }
        cout << "                }" << endl;

        // input tiles from other channels
        for( unsigned j = _useDestination ? 0 : 1; j<_nChannels; ++j )
        {
            if( i == j ) // own tile, is in place
                continue;

            cout << "                inputframe{ name \"tile" << i << ".channel"
                 << j <<"\" }" << endl;
        }

        // assembled color tile output, if not already in place
        if( i != 0 )
        {
            cout << "                outputframe{ name \"frame.channel" << i <<"\"";
            cout << " buffer [ COLOR ] ";
            if( i == _nChannels - 1 ) // last - correct rounding 'error'
                cout << "viewport [ 0 0." << setw(5) << start << " 1 0." << setw(5)
                     << 100000 - start << " ]";
            else
                cout << "viewport [ 0 0." << setw(5) << start << " 1 0." << setw(5)
                     << step << " ]";
            cout << " }" << endl;
        }
        cout << "            }" << endl
             << endl;

        start += step;
    }

    // gather assembled input tiles
    for( unsigned i = 1; i<_nChannels; ++i )
        cout << "            inputframe{ name \"frame.channel" << i << "\" }" 
             << endl;
    cout << "        }" << endl;    
}


void ConfigTool::_writeDSAC() const
{
    cout << "        compound" << endl
         << "        {" << endl
         << "            channel   \"channel0\"" << endl
         << "            buffer    [ COLOR DEPTH ]" << endl
         << "            wall" << endl
         << "            {" << endl
         << "                bottom_left  [ -.32 -.20 -.75 ]" << endl
         << "                bottom_right [  .32 -.20 -.75 ]" << endl
         << "                top_left     [ -.32  .20 -.75 ]" << endl
         << "            }" << endl;

    unsigned nChannels = _nChannels / 2;
    unsigned start      = 0;
    const unsigned step = static_cast< unsigned >( 100000.0f / nChannels );
    for( unsigned i = 0; i<nChannels; ++i, start += step )
    {
        // Rendering compund
        cout << "            compound" << endl
             << "            {" << endl
             << "                channel   \"channel" << i*2+1 << "\"" << endl;

        // leaf draw + tile readback compound
        cout << "                compound" << endl
             << "                {" << endl;
        if( i == nChannels - 1 ) // last - correct rounding 'error'
            cout << "                    range     [ 0." << setw(5) << start 
                 << " 1 ]" << endl;
        else
            cout << "                    range     [ 0." << setw(5) << start 
                 << " 0." << setw(5) << start + step << " ]" << endl;
        
        unsigned y = 0;
        for( unsigned j = 0; j<nChannels; ++j )
        {
            cout << "                    outputframe{ name \"tile" << j*2+1 
                 << ".channel" << i*2+1 << "\" ";
            if( j == nChannels - 1 ) // last - correct rounding 'error'
                cout << "viewport [ 0 0." << setw(5) << y << " 1 0." << setw(5)
                     << 100000-y << " ]";
            else
                cout << "viewport [ 0 0." << setw(5) << y << " 1 0." << setw(5)
                     << step << " ]";
            cout << " }" << endl;

            y += step;
        }
        cout << "                }" << endl
             << "            }" << endl
             << endl;

        // compositing compund
        cout << "            compound" << endl
             << "            {" << endl
             << "                channel   \"channel" << i*2 << "\"" << endl;

        // input tiles from other channels
        for( unsigned j = 0; j<nChannels; ++j )
        {
            cout << "                inputframe{ name \"tile" << i*2+1 << ".channel"
                 << j*2+1 <<"\" }" << endl;
        }

        // assembled color tile output, if not already in place
        if( i!=0 )
        {
            cout << "                outputframe{ name \"frame.channel" << i*2 <<"\"";
            cout << " buffer [ COLOR ] ";
            if( i == nChannels - 1 ) // last - correct rounding 'error'
                cout << "viewport [ 0 0." << setw(5) << start << " 1 0." << setw(5)
                     << 100000 - start << " ]";
            else
                cout << "viewport [ 0 0." << setw(5) << start << " 1 0." << setw(5)
                     << step << " ]";
            cout << " }" << endl;
        }


        cout << "            }" << endl
             << endl;
    }

    // gather assembled input tiles
    for( unsigned i = 1; i<nChannels; ++i )
        cout << "            inputframe{ name \"frame.channel" << i*2 << "\" }" 
             << endl;
    cout << "        }" << endl;    
}

void ConfigTool::_writeWall() const
{
    cout << "        compound" << endl
         << "        {" << endl;

    const float screenW =  0.60;       //screen width in meters
    const float screenH =  0.48;       //screen height in meters
    const float screenD = -0.41*_wallH; //screen distance to viewer in meters

    float utmostLeft = -screenW*_wallW / 2;
    float utmostLow  = -screenH*_wallH / 2;

    cout << setiosflags(ios::fixed) << setprecision(5);

    for( uint h = 0; h < _wallH; h++ )
        for( uint w = 0; w < _wallW; w++ )
        {
            float b = utmostLow  + screenH*h; //bottom
            float l = utmostLeft + screenW*w; //left
            float r = l + screenW;            //right
            float t = b + screenH;            //top
            float d = screenD;                //depth
            cout
            << "            compound" << endl
            << "            {"        << endl
            << "                channel   \"channel" << h*_wallW + w << "\"" << endl
            << "                wall" << endl
            << "                {" << endl
            << "                    bottom_left  [ "<< l <<" "<< b <<" "<< d <<"]" << endl
            << "                    bottom_right [ "<< r <<" "<< b <<" "<< d <<"]" << endl
            << "                    top_left     [ "<< l <<" "<< t <<" "<< d <<"]" << endl
            << "                }" << endl
            << "            }" << endl;
        }

    cout << "        }" << endl;
}

