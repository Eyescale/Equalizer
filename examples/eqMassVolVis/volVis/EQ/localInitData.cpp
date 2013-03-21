
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#include "localInitData.h"
#include "volVis.h"

#include <msv/util/statLogger.h>

#include <msv/util/str.h>

#ifndef MIN
#  define MIN EQ_MIN
#endif
#include <tclap/CmdLine.h>

#include <algorithm>
#include <cctype>
#include <functional>

#ifdef GLX
#  include <X11/Xlib.h>
#endif

namespace massVolVis
{


LocalInitData::LocalInitData()
        : _maxFrames( 0xffffffffu )
        , _isResident( false )
        , _ortho( false )
//        , _rType( SLICE )
        , _rType( RAYCAST )
        , _dumpStatistics( false )
{}

const LocalInitData& LocalInitData::operator = ( const LocalInitData& from )
{
    _maxFrames      = from._maxFrames;
    _isResident     = from._isResident;
    _ortho          = from._ortho;
    _rType          = from._rType;
    _pathFilename   = from._pathFilename;
    _dumpStatistics = from._dumpStatistics;

    setFilename( from.getFilename( ));
    setWindowSystem( from.getWindowSystem( ));
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

        TCLAP::CmdLine command( VolVis::getHelp() );

        TCLAP::ValueArg<std::string> modelArg( "m", "model",
                                               "dat model file name",
                                               false, "Bucky.dat",
                                               "string", command );

        TCLAP::ValueArg<std::string> rendererArg( "d", "drawer",
                                               "rendering algorithm",
                                               false, "slice",
                                               "string", command );

        TCLAP::SwitchArg residentArg( "r", "resident",
           "Keep client resident (see resident node documentation on website)",
                                      command, false );
        TCLAP::ValueArg<uint32_t> framesArg( "n", "numFrames",
                                           "Maximum number of rendered frames",
                                             false, 0xffffffffu, "unsigned",
                                             command );
        TCLAP::ValueArg<std::string> wsArg( "w", "windowSystem", wsHelp,
                                            false, "auto", "string", command );

        TCLAP::ValueArg<std::string> pathArg( "a", "cameraPath",
                                        "File containing camera path animation",
                                              false, "", "string", command );

        TCLAP::ValueArg<std::string> logArg( "l", "logFilePath",
                                        "File for dumping timings log (use 'print' for console output )",
                                              false, "", "string", command );

        command.parse( argc, argv );

        if( modelArg.isSet( ))
            setFilename( modelArg.getValue( ));

        if( wsArg.isSet( ))
        {
            const std::string windowSystem = strUtil::toLower( wsArg.getValue( ));

            if( windowSystem == "glx" )
                setWindowSystem( eq::WINDOW_SYSTEM_GLX );
            else if( windowSystem == "agl" )
                setWindowSystem( eq::WINDOW_SYSTEM_AGL );
            else if( windowSystem == "wgl" )
                setWindowSystem( eq::WINDOW_SYSTEM_WGL );
        }

#ifdef GLX
        bool useGLX = getWindowSystem() == eq::WINDOW_SYSTEM_GLX;
#ifndef AGL
        useGLX = true;
#endif
        if( useGLX )
        {
            LBWARN << "Using GLX" << std::endl;
#ifndef __APPLE__
            XInitThreads();
#endif
        }else
#endif
        LBWARN << "Not using GLX" << std::endl;

        if( rendererArg.isSet( ))
        {
            const std::string renderer = strUtil::toLower( rendererArg.getValue( ));
            if( renderer == "slice" )
                _rType = SLICE;
        }

        if( framesArg.isSet( ))
            _maxFrames = framesArg.getValue();
        if( residentArg.isSet( ))
            _isResident = true;

        if( pathArg.isSet( ))
            _pathFilename = pathArg.getValue();

        if( logArg.isSet() )
        {
            _dumpStatistics = true;
            if( strUtil::toLower( logArg.getValue() ) != "print" )
                util::StatLogger::instance().setLogFilePath( logArg.getValue() );
        }
    }
    catch( const TCLAP::ArgException& exception )
    {
        LBERROR << "Command line parse error: " << exception.error()
                << " for argument " << exception.argId() << std::endl;
        ::exit( EXIT_FAILURE );
    }
}

}//namespace massVolVis

