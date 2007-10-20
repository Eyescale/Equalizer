
/*
 * Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved. 
 */

#include "localInitData.h"
#include "frameData.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <tclap/CmdLine.h>

using namespace std;

namespace eVolve
{
LocalInitData::LocalInitData()
        : _maxFrames( 0xffffffffu )
        , _isResident( false )
{}

const LocalInitData& LocalInitData::operator = ( const LocalInitData& from )
{
    _maxFrames   = from._maxFrames;    
    _isResident  = from._isResident;
    setFilename( from.getFilename( ));
    setWindowSystem( from.getWindowSystem( ));
    setPrecision( from.getPrecision( ));
    setBrightness( from.getBrightness( ));
    if( from.useGLSL( )) 
      enableGLSL();
    return *this;
}

void LocalInitData::parseArguments( const int argc, char** argv )
{
    try
    {
        string wsHelp = "Window System API ( one of: ";
#ifdef AGL
        wsHelp += "AGL ";
#endif
#ifdef GLX
        wsHelp += "glX ";
#endif
#ifdef WGL
        wsHelp += "WGL ";
#endif
        wsHelp += ")";


        TCLAP::CmdLine command( "eVolve - Equalizer volume rendering example" );
        
        TCLAP::ValueArg<string> modelArg( "m", "model", "raw model file name",
                                          false, "Bucky32x32x32.raw", "string",
                                          command );
        TCLAP::SwitchArg residentArg( "r", "resident", 
           "Keep client resident (see resident node documentation on website)", 
                                      command, false );
        TCLAP::ValueArg<uint32_t> framesArg( "n", "numFrames", 
                                           "Maximum number of rendered frames", 
                                             false, 0xffffffffu, "unsigned",
                                             command );
        TCLAP::ValueArg<uint32_t> precisionArg( "p", "precision", 
                "Rendering precision (default 2, bigger is better and slower)", 
                                                false, 2, "unsigned",
                                                command );
        TCLAP::ValueArg<float> brightnessArg( "b", "brightness",
                                              "brightness factor", false, 1.0f,
                                              "float", command );
        TCLAP::ValueArg<string> wsArg( "w", "windowSystem", wsHelp,
                                       false, "auto", "string", command );
        TCLAP::SwitchArg glslArg( "g", "glsl", "Enable GLSL shaders", 
                                  command, false );
        
        command.parse( argc, argv );

        if( modelArg.isSet( ))
            setFilename( modelArg.getValue( ));
        if( wsArg.isSet( ))
        {
            string windowSystem = wsArg.getValue();
            transform( windowSystem.begin(), windowSystem.end(),
                       windowSystem.begin(), (int(*)(int))std::tolower );

            if( windowSystem == "glx" )
                setWindowSystem( eq::WINDOW_SYSTEM_GLX );
            else if( windowSystem == "agl" )
                setWindowSystem( eq::WINDOW_SYSTEM_AGL );
            else if( windowSystem == "wgl" )
                setWindowSystem( eq::WINDOW_SYSTEM_WGL );
        }

        if( framesArg.isSet( ))
            _maxFrames = framesArg.getValue();
        if( precisionArg.isSet( ))
            setPrecision( precisionArg.getValue( ));
        if( brightnessArg.isSet( ))
            setBrightness( brightnessArg.getValue( ));
        if( residentArg.isSet( ))
            _isResident = true;
        if( glslArg.isSet() )
            enableGLSL();
    }
    catch( TCLAP::ArgException& exception )
    {
        EQERROR << "Command line parse error: " << exception.error() 
                << " for argument " << exception.argId() << endl;
    }
}
}

