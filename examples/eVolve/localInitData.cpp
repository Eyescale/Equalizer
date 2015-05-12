
/* Copyright (c) 2007-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#pragma warning( disable: 4275 )
#include <boost/program_options.hpp>
#pragma warning( default: 4275 )

#ifndef MIN
#  define MIN LB_MIN
#endif

namespace po = boost::program_options;

namespace eVolve
{
LocalInitData::LocalInitData()
        : _maxFrames( 0xffffffffu )
        , _isResident( false )
        , _ortho( false )
{}

LocalInitData& LocalInitData::operator = ( const LocalInitData& from )
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
    std::string wsHelp = "Window System API ( one of: ";
#ifdef AGL
    wsHelp += "AGL ";
#endif
#ifdef GLX
    wsHelp += "GLX ";
#endif
#ifdef WGL
    wsHelp += "WGL ";
#endif
#ifdef EQUALIZER_USE_QT5WIDGETS
    wsHelp += "Qt ";
#endif
    wsHelp += ")";

    bool showHelp(false);
    uint32_t userDefinedPrecision(2);
    float userDefinedBrightness(1.0f);
    float userDefinedAlpha(1.0f);
    std::string userDefinedModelPath("");
    std::string userDefinedWindowSystem("");

    const std::string& desc = EVolve::getHelp();
    po::options_description options( desc + " Version " +
                                     eq::Version::getString( ));
    options.add_options()
        ( "help,h",       po::bool_switch(&showHelp)->default_value(false),
          "produce help message" )
        ( "model,m",      po::value<std::string>(&userDefinedModelPath),
          "raw model file name, e.g. Bucky32x32x32.raw" )
        ( "resident,r", po::bool_switch(&_isResident)->default_value(false),
          "Keep client resident (see resident mode documentation on website)" )
        ( "numFrames,n",
          po::value<uint32_t>(&_maxFrames)->default_value(0xffffffffu),
          "Maximum number of rendered frames")
        ( "precision,p",
          po::value<uint32_t>(&userDefinedPrecision)->default_value(2),
          "Rendering precision (default 2, bigger is better and slower)")
        ( "brightness,b",
          po::value<float>(&userDefinedBrightness)->default_value(1.0f),
          "brightness factor" )
        ( "alpha,a",
          po::value<float>(&userDefinedAlpha)->default_value(1.0f),
          "alpha attenuation" )
        ( "ortho,o",      po::bool_switch(&_ortho)->default_value(false),
          "use orthographic projection" )
        ( "windowSystem,w",
          po::value<std::string>(&userDefinedWindowSystem),
          wsHelp.c_str( ));

    po::variables_map variableMap;
    try
    {
        // parse program options, ignore non related options
        po::store( po::command_line_parser( argc, argv ).options(
                       options ).allow_unregistered().run(),
                   variableMap );
        po::notify(variableMap);
    }
    catch( std::exception& exception )
    {
        LBERROR << "Error parsing command line: " << exception.what()
                << std::endl;
        eq::exit(); // cppcheck-suppress unreachableCode
        ::exit( EXIT_FAILURE );
    }

    // Evaluate parsed command line options
    if( showHelp )
    {
        std::cout << options << std::endl;
        eq::exit(); // cppcheck-suppress unreachableCode
        ::exit( EXIT_SUCCESS );
    }

    if ( variableMap.count( "model" ) > 0 )
        setFilename( userDefinedModelPath );

    if( variableMap.count( "windowSystem" ) > 0 )
        setWindowSystem( userDefinedWindowSystem );


    if( variableMap.count("precision") > 0 )
        setPrecision( userDefinedPrecision );
    if( variableMap.count("brightness") > 0 )
        setBrightness( userDefinedBrightness );
    if( variableMap.count("alpha") > 0 )
        setAlpha( userDefinedAlpha );
}

}
