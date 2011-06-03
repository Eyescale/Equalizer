
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

        std::string desc = EVolve::getHelp();

        TCLAP::CmdLine command( desc );
        
        TCLAP::ValueArg<std::string> modelArg( "m", "model", 
                                               "raw model file name",
                                               false, "Bucky32x32x32.raw",
                                               "string", command );
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
        TCLAP::ValueArg<std::string> wsArg( "w", "windowSystem", wsHelp,
                                            false, "auto", "string", command );
        TCLAP::VariableSwitchArg ignoreEqArgs( "eq",
                                               "Ignored Equalizer options",
                                               command );
        TCLAP::UnlabeledMultiArg< std::string >
            ignoreArgs( "ignore", "Ignored unlabeled arguments", false, "any",
                        command );

        command.parse( argc, argv );

        if( modelArg.isSet( ))
            setFilename( modelArg.getValue( ));
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
    catch( const TCLAP::ArgException& exception )
    {
        EQERROR << "Command line parse error: " << exception.error() 
                << " for argument " << exception.argId() << std::endl;
        ::exit( EXIT_FAILURE );
    }
}
}

