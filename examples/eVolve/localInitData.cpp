/*
 * Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved. 
 */

#include "localInitData.h"
#include "frameData.h"

#include <algorithm>
#include <cctype>
#include <functional>

#ifndef MIN
#  define MIN EQ_MIN
#endif
#include <tclap/CmdLine.h>

using namespace std;

namespace eVolve
{
LocalInitData::LocalInitData()
        : _maxFrames( 0xffffffffu )
        , _isResident( false )
        , _ortho( false )
{}

const LocalInitData& LocalInitData::operator = ( const LocalInitData& from )
{
    _maxFrames   = from._maxFrames;    
    _isResident  = from._isResident;
    _ortho       = from._ortho;

    setFilename( from.getFilename( ));
    setWindowSystem( from.getWindowSystem( ));
    setPrecision( from.getPrecision( ));
    setBrightness( from.getBrightness( ));
    setAlpha( from.getAlpha( ));
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

        string desc = 
            string( "eVolve - Equalizer volume rendering example\n" ) +
            string( "\tRun-time commands:\n" ) +
            string( "\t\tLeft Mouse Button:         Rotate model\n" ) +
            string( "\t\tMiddle Mouse Button:       Move model in X, Y\n" ) +
            string( "\t\tRight Mouse Button:        Move model in Z\n" ) +
            string( "\t\t<Esc>, All Mouse Buttons:  Exit program\n" ) +
            string( "\t\t<Space>, r:                Reset camera\n" ) +
            string( "\t\to:                         Toggle perspective/orthographic\n" );
            string( "\t\ts:                         Toggle statistics overlay\n" );

        TCLAP::CmdLine command( desc );
        
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
        TCLAP::ValueArg<float> alphaArg( "a", "alpha", "alpha attenuation", 
                                         false, 1.0f, "float", command );
        TCLAP::SwitchArg orthoArg( "o", "ortho", 
                                   "use orthographic projection", 
                                   command, false );
        TCLAP::ValueArg<string> wsArg( "w", "windowSystem", wsHelp,
                                       false, "auto", "string", command );

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
        if( alphaArg.isSet( ))
            setAlpha( alphaArg.getValue( ));
        if( residentArg.isSet( ))
            _isResident = true;
        if( orthoArg.isSet( ))
            _ortho = true;
    }
    catch( TCLAP::ArgException& exception )
    {
        EQERROR << "Command line parse error: " << exception.error() 
                << " for argument " << exception.argId() << endl;
    }
}
}

