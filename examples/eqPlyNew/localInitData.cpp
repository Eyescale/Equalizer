
/*
 * Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved. 
 */

#include "localInitData.h"
#include "frameData.h"

#include <tclap/CmdLine.h>

using namespace std;

namespace eqPly
{
LocalInitData::LocalInitData()
        : _maxFrames( 0xffffffffu ),
          _clientPort( 0 ),
          _color( true ),
          _isApplication( true )
{}

void LocalInitData::parseArguments( int argc, char** argv )
{
    try
    {
        TCLAP::CmdLine command( "eqPly - Equalizer polygonal rendering example" );
        TCLAP::ValueArg<string> modelArg( "m", "model", "ply model file name", 
                                          false, "rockerArm.ply", "string", 
                                          command );
        TCLAP::ValueArg<string> portArg( "p", "port", "tracking device port",
                                         false, "/dev/ttyS0", "string", 
                                         command );
        TCLAP::SwitchArg colorArg( "b", "bw", "Don't use colors from ply file", 
                                   command, false );
        TCLAP::ValueArg<uint32_t> framesArg( "n", "numFrames", 
                                           "Maximum number of rendered frames", 
                                             false, 0xffffffffu, "unsigned",
                                             command );
        TCLAP::ValueArg<uint16_t> clientArg( "c", "client", 
                                             "Run as resident render client", 
                                             false, 4243, "unsigned short",
                                             command );
                                
        command.parse( argc, argv );

        if( modelArg.isSet( ))
            setFilename( modelArg.getValue( ));
        if( portArg.isSet( ))
            _trackerPort = portArg.getValue();

        _color         = !colorArg.isSet();

        if( framesArg.isSet( ))
            _maxFrames = framesArg.getValue();

        if( clientArg.isSet( ))
        {
            _isApplication = false;
            _clientPort    = clientArg.getValue();
        }
    }
    catch( TCLAP::ArgException& exception )
    {
        EQERROR << "Command line parse error: " << exception.error() 
                << " for argument " << exception.argId() << endl;
    }
}
}

