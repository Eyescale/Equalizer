
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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

namespace eqPly
{
LocalInitData::LocalInitData() 
        : _maxFrames( 0xffffffffu )
        , _color( true )
        , _isResident( false )
{
    _filenames.push_back( "examples/eqPly/" );
    _filenames.push_back( "../examples/eqPly/" );
    _filenames.push_back( "./../../../examples/eqPly" ); 
    _filenames.push_back( "../../../../examples/eqPly" );
    _filenames.push_back( "../share/data/" );
    _filenames.push_back( "/opt/local/share/data/" );
    _filenames.push_back( "/usr/local/share/data/" );
    _filenames.push_back( "/usr/share/data/" );
}

const LocalInitData& LocalInitData::operator = ( const LocalInitData& from )
{
    _trackerPort = from._trackerPort;  
    _maxFrames   = from._maxFrames;    
    _color       = from._color;        
    _isResident  = from._isResident;
    _filenames    = from._filenames;
    _pathFilename = from._pathFilename;

    setWindowSystem( from.getWindowSystem( ));
    setRenderMode( from.getRenderMode( ));
    if( from.useGLSL( )) 
        enableGLSL();
    if( from.useInvertedFaces( )) 
        enableInvertedFaces();
    if( !from.showLogo( )) 
        disableLogo();

    return *this;
}

void LocalInitData::parseArguments( const int argc, char** argv )
{
    try
    {
        std::string wsHelp = "Window System API ( one of: ";
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

        const std::string& desc = EqPly::getHelp();

        TCLAP::CmdLine command( desc );
        TCLAP::MultiArg<std::string> modelArg( "m", "model", 
                                             "ply model file name or directory",
                                               false, "string", command );
        TCLAP::ValueArg<std::string> portArg( "p", "port",
                                              "tracking device port",
                                              false, "/dev/ttyS0", "string",
                                              command );
        TCLAP::SwitchArg colorArg( "b", "blackAndWhite", 
                                   "Don't use colors from ply file", 
                                   command, false );
        TCLAP::SwitchArg residentArg( "r", "resident", 
           "Keep client resident (see resident node documentation on website)", 
                                      command, false );
        TCLAP::ValueArg<uint32_t> framesArg( "n", "numFrames", 
                                           "Maximum number of rendered frames", 
                                             false, 0xffffffffu, "unsigned",
                                             command );
        TCLAP::ValueArg<std::string> wsArg( "w", "windowSystem", wsHelp,
                                            false, "auto", "string", command );
        TCLAP::ValueArg<std::string> modeArg( "c", "renderMode", 
                                 "Rendering Mode (immediate, displayList, VBO)",
                                              false, "auto", "string",
                                              command );
        TCLAP::SwitchArg glslArg( "g", "glsl", "Enable GLSL shaders", 
                                    command, false );
        TCLAP::SwitchArg invFacesArg( "i", "invertFaces",
                             "Invert faces (valid during binary file creation)",
                                    command, false );
        TCLAP::ValueArg<std::string> pathArg( "a", "cameraPath",
                                        "File containing camera path animation",
                                              false, "", "string", command );
        TCLAP::SwitchArg overlayArg( "o", "noOverlay", "Disable overlay logo", 
                                     command, false );
        TCLAP::VariableSwitchArg ignoreEqArgs( "eq",
                                               "Ignored Equalizer options",
                                               command );
        TCLAP::UnlabeledMultiArg< std::string >
            ignoreArgs( "ignore", "Ignored unlabeled arguments", false, "any",
                        command );

        command.parse( argc, argv );

        if( modelArg.isSet( ))
        {
            _filenames.clear();
            _filenames = modelArg.getValue();
        }
        if( portArg.isSet( ))
            _trackerPort = portArg.getValue();
        if( wsArg.isSet( ))
        {
            std::string windowSystem = wsArg.getValue();
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
            std::string mode = modeArg.getValue();
            transform( mode.begin(), mode.end(), mode.begin(),
                       (int(*)(int))std::tolower );
            
            if( mode == "immediate" )
                setRenderMode( mesh::RENDER_MODE_IMMEDIATE );
            else if( mode == "displaylist" )
                setRenderMode( mesh::RENDER_MODE_DISPLAY_LIST );
            else if( mode == "vbo" )
                setRenderMode( mesh::RENDER_MODE_BUFFER_OBJECT );
        }

        if( pathArg.isSet( ))
            _pathFilename = pathArg.getValue();

        if( glslArg.isSet() )
            enableGLSL();
        if( invFacesArg.isSet() )
            enableInvertedFaces();
        if( overlayArg.isSet( ))
            disableLogo();
    }
    catch( TCLAP::ArgException& exception )
    {
        EQERROR << "Command line parse error: " << exception.error() 
                << " for argument " << exception.argId() << std::endl;
        ::exit( EXIT_FAILURE );
    }
}
}

