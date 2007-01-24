
/*
 * Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved. 
 */

#include "appInitData.h"

#include <tclap/CmdLine.h>

void AppInitData::parseArguments( int argc, char** argv )
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
                                
        command.parse( argc, argv );

        if( modelArg.isSet( ))
            setFilename( modelArg.getValue( ));
        if( portArg.isSet( ))
            _trackerPort = portArg.getValue();
    }
    catch( TCLAP::ArgException& exception )
    {
        EQERROR << "Command line parse error: " << exception.error() 
                << " for argument " << exception.argId() << endl;
    }
}
