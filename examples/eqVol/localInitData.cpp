
/*
 * Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved. 
 */

#include "localInitData.h"
#include "frameData.h"

#include <tclap/CmdLine.h>
#include "rawConverter.h"


using namespace std;

namespace eqVol
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
        TCLAP::CmdLine command( "eqVol - Equalizer volume rendering example" );
        TCLAP::ValueArg<string> modelArg( "m", "model", "raw model file name", 
                                          false, "Bucky32x32x32.raw", "string", 
                                          command );
         TCLAP::ValueArg<string> derArg( "d", "der", "data plus derivatives name", 
                                          false, "Bucky32x32x32_d.raw", "string", 
                                          command );
         TCLAP::ValueArg<string> savArg( "s", "sav", "sav to vhf transfer function converter", 
                                          false, "Bucky32x32x32.raw.vhf", "string", 
                                          command );
//         TCLAP::ValueArg<string> infoArg( "i", "inf", "info file name", 
//                                          false, "Bucky32x32x32.inf", "string", 
//                                          command );
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
            setDataFilename( modelArg.getValue( ));

        if( derArg.isSet( ))
		{
            ConvertRawToRawPlusDerivatives( getDataFilename(), derArg.getValue( ));
			exit(0);
		}

        if( savArg.isSet( ))
		{
            SavToVhfConverter( getDataFilename(), savArg.getValue( ));
			exit(0);
		}

//        if( infoArg.isSet( ))
//            setInfoFilename( infoArg.getValue( ));

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

