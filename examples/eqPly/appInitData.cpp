
/*
 * Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved. 
 */

#include "appInitData.h"
#include "frameData.h"

#include <tclap/CmdLine.h>

void AppInitData::parseArguments( int argc, char** argv, FrameData& frameData )
{
    try
    {
        TCLAP::CmdLine command("eqPly - Equalizer polygonal rendering example");
        TCLAP::ValueArg<string> modelArg( "m", "model", "ply model file name", 
                                          false, "rockerArm.ply", "string", 
                                          command );
        TCLAP::ValueArg<string> portArg( "p", "port", "tracking device port",
                                         false, "/dev/ttyS0", "string", 
                                         command );
        TCLAP::SwitchArg colorArg( "b", "bw", "Don't use colors from ply file", 
                                   command, false );
                                
        command.parse( argc, argv );

        if( modelArg.isSet( ))
            setFilename( modelArg.getValue( ));
        if( portArg.isSet( ))
            _trackerPort = portArg.getValue();

        frameData.data.color = !colorArg.isSet();
    }
    catch( TCLAP::ArgException& exception )
    {
        EQERROR << "Command line parse error: " << exception.error() 
                << " for argument " << exception.argId() << endl;
    }
}
