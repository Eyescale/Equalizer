
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "configTool.h"

#ifdef WIN32_VC
#  define MIN __min
#else
#  include <sys/param.h>
#endif

#include <math.h>
#include <tclap/CmdLine.h>

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
          _useDestination( true )
{}

bool ConfigTool::parseArguments( int argc, char** argv )
{
    try
    {
        TCLAP::CmdLine command( "configTool - Equalizer config file generator" );
        TCLAP::ValueArg<string> modeArg( "m", "mode", "Compound mode (default 2D)",
                                         false, "2D", "2D|DB|DB_ds", command );
        TCLAP::ValueArg<unsigned> pipeArg( "p", "numPipes", 
                                           "Number of pipes per node (default 1)",
                                           false, 1, "unsigned", command );
        TCLAP::ValueArg<unsigned> channelArg( "c", "numChannels", 
                                             "Total number of channels (default 4)",
                                              false, 4, "unsigned", command );
        TCLAP::SwitchArg destArg( "a", "assembleOnly", 
                            "Destination channel does not contribute to rendering", 
                                  command, false );
                                
        command.parse( argc, argv );

        if( modeArg.isSet( ))
        {
            const string& mode = modeArg.getValue();
            if( mode == "2D" )
                _mode = MODE_2D;
            else if( mode == "DB" )
                _mode = MODE_DB;
            else if( mode == "DB_ds" )
                _mode = MODE_DB_DS;
            else
            {
                cerr << "Unknown mode " << mode << endl;
                return false;
            }
        }

        if( pipeArg.isSet( ))
            _nPipes = pipeArg.getValue();
        if( channelArg.isSet( ))
            _nChannels = channelArg.getValue();

        _useDestination = !destArg.isSet();
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

void ConfigTool::_writeResources() const
{
    const unsigned nNodes  = _nChannels/_nPipes + 1;
    unsigned       channel = 0;
    for( unsigned node=0; node < nNodes && channel < _nChannels; ++node )
    {
        cout << "        node" << endl
             << "        {" << endl
             << "            name       \"node" << node << "\""<< endl
             << "            connection { hostname \"node" << node << "\" }"<< endl;
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
            if( channel == 0 ) // destination window
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

        default:
            cerr << "Unimplemented mode " << _mode << endl;
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
            cout << "                viewport  [ 0 0." << setw(5) << y << " 1 0."
                 << setw(5) << 100000-y << " ]" << endl;
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
