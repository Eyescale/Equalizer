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

namespace eqPly
{
LocalInitData::LocalInitData() :
#ifdef WIN32_VC
        _filename( "../examples/eqPly/rockerArm.ply" )
#else
        _filename( "../share/data/rockerArm.ply" )
#endif
        , _maxFrames( 0xffffffffu )
        , _color( true )
        , _isResident( false )
{}

const LocalInitData& LocalInitData::operator = ( const LocalInitData& from )
{
    _trackerPort = from._trackerPort;  
    _maxFrames   = from._maxFrames;    
    _color       = from._color;        
    _isResident  = from._isResident;
    _filename    = from._filename;

    setWindowSystem( from.getWindowSystem( ));
    setRenderMode( from.getRenderMode( ));
    if( from.useGLSL( )) 
        enableGLSL();
    if( from.useInvertedFaces( )) 
        enableInvertedFaces();
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
            string( "eqPly - Equalizer polygonal rendering example\n" ) +
            string( "\tRun-time commands:\n" ) +
            string( "\t\tLeft Mouse Button:         Rotate model\n" ) +
            string( "\t\tMiddle Mouse Button:       Move model in X, Y\n" ) +
            string( "\t\tRight Mouse Button:        Move model in Z\n" ) +
            string( "\t\t<Cursor Keys>:             Move head in X,Y plane\n" )+
            string( "\t\t<Page Up,Down>:            Move head in Z\n" )+
            string( "\t\t<Esc>, All Mouse Buttons:  Exit program\n" ) +
            string( "\t\t<Space>, r:                Reset camera\n" ) +
            string( "\t\to:                         Toggle perspective/orthographic\n" ) +
            string( "\t\ts:                         Toggle statistics overlay\n" ) +
            string( "\t\tw:                         Toggle wireframe mode\n" ) +
            string( "\t\tm:                         Switch rendering mode\n" );

        TCLAP::CmdLine command( desc );
        TCLAP::ValueArg<string> modelArg( "m", "model", "ply model file name", 
                                          false, "rockerArm.ply", "string", 
                                          command );
        TCLAP::ValueArg<string> portArg( "p", "port", "tracking device port",
                                         false, "/dev/ttyS0", "string",
                                         command );
        TCLAP::SwitchArg colorArg( "b", "bw", "Don't use colors from ply file", 
                                   command, false );
        TCLAP::SwitchArg residentArg( "r", "resident", 
           "Keep client resident (see resident node documentation on website)", 
                                      command, false );
        TCLAP::ValueArg<uint32_t> framesArg( "n", "numFrames", 
                                           "Maximum number of rendered frames", 
                                             false, 0xffffffffu, "unsigned",
                                             command );
        TCLAP::ValueArg<string> wsArg( "w", "windowSystem", wsHelp,
                                       false, "auto", "string", command );
        TCLAP::ValueArg<string> modeArg( "c", "renderMode", 
                                 "Rendering Mode (immediate, displayList, VBO)",
                                       false, "auto", "string", command );
        TCLAP::SwitchArg glslArg( "g", "glsl", "Enable GLSL shaders", 
                                    command, false );
        TCLAP::SwitchArg invFacesArg( "i", "iface",
            "Invert faces (valid during binary file creation)", 
                                    command, false );
        
        command.parse( argc, argv );

        if( modelArg.isSet( ))
            _filename = modelArg.getValue();
        if( portArg.isSet( ))
            _trackerPort = portArg.getValue();
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

        _color = !colorArg.isSet();

        if( framesArg.isSet( ))
            _maxFrames = framesArg.getValue();

        if( residentArg.isSet( ))
            _isResident = true;
        
        if( modeArg.isSet() )
        {
            string mode = modeArg.getValue();
            transform( mode.begin(), mode.end(), mode.begin(),
                       (int(*)(int))std::tolower );
            
            if( mode == "immediate" )
                setRenderMode( mesh::RENDER_MODE_IMMEDIATE );
            else if( mode == "displaylist" )
                setRenderMode( mesh::RENDER_MODE_DISPLAY_LIST );
            else if( mode == "vbo" )
                setRenderMode( mesh::RENDER_MODE_BUFFER_OBJECT );
        }

        if( glslArg.isSet() )
            enableGLSL();
        if( invFacesArg.isSet() )
            enableInvertedFaces();
    }
    catch( TCLAP::ArgException& exception )
    {
        EQERROR << "Command line parse error: " << exception.error() 
                << " for argument " << exception.argId() << endl;
    }
}
}

