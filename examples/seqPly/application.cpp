
/* Copyright (c) 2011-2013, Stefan Eilemann <eile@eyescale.ch>
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

#include "application.h"

#include "renderer.h"

#pragma warning( disable: 4275 )
#include <boost/program_options.hpp>
#pragma warning( default: 4275 )

#ifndef MIN
#  define MIN LB_MIN
#endif

namespace po = boost::program_options;

namespace seqPly
{
bool Application::init( const int argc, char** argv )
{
    const eq::Strings& models = _parseArguments( argc, argv );
    if( !seq::Application::init( argc, argv, 0 ))
        return false;

    _loadModel( models );
    return true;
}

bool Application::run()
{
    return seq::Application::run( &_frameData );
}

bool Application::exit()
{
    _unloadModel();
    return seq::Application::exit();
}

seq::Renderer* Application::createRenderer()
{
    return new Renderer( *this );
}

co::Object* Application::createObject( const uint32_t type )
{
    switch( type )
    {
      case seq::OBJECTTYPE_FRAMEDATA:
          return new eqPly::FrameData;

      default:
          return seq::Application::createObject( type );
    }
}

namespace
{
static bool _isPlyfile( const std::string& filename )
{
    const size_t size = filename.length();
    if( size < 5 )
        return false;

    if( filename[size-4] != '.' || filename[size-3] != 'p' ||
        filename[size-2] != 'l' || filename[size-1] != 'y' )
    {
        return false;
    }
    return true;
}
}

eq::Strings Application::_parseArguments( const int argc, char** argv )
{
    std::string userDefinedModelPath("");

    try //parse command line arguments
    {
        po::options_description options(
            std::string("seqPly - Sequel polygonal rendering example ")
            + eq::Version::getString( ));
        bool showHelp(false);

        options.add_options()
            ( "help,h", po::bool_switch(&showHelp)->default_value(false),
              "produce help message" )
            ( "model,m", po::value<std::string>(&userDefinedModelPath),
              "ply model file name" );

        // parse program options, ignore all non related options
        po::variables_map variableMap;
        po::store( po::command_line_parser( argc, argv ).options(
                       options ).allow_unregistered().run(),
                   variableMap );
        po::notify( variableMap );

        // Evaluate parsed command line options
        if( showHelp )
        {
            LBWARN << options << std::endl;
            ::exit( EXIT_SUCCESS );
        }
    }
    catch( std::exception& exception )
    {
        LBERROR << "Error parsing command line: " << exception.what()
            << std::endl;
    }


    eq::Strings filenames;
    filenames.push_back( lunchbox::getRootPath() +
                         "/share/Equalizer/data" );

    if( !userDefinedModelPath.empty( ))
    {
        filenames.clear();
        filenames.push_back( userDefinedModelPath );
    }
    return filenames;
}

void Application::_loadModel( const eq::Strings& models )
{
    eq::Strings files = models;
    while( !files.empty( ))
    {
        const std::string filename = files.back();
        files.pop_back();

        if( _isPlyfile( filename ))
        {
            _model = new Model;
            if( _model->readFromFile( filename.c_str( )))
            {
                _modelDist = new ModelDist( _model );
                _modelDist->registerTree( this );
                _frameData.setModelID( _modelDist->getID( ));
                return;
            }
            delete _model;
            _model = 0;
        }
        else
        {
            const std::string basename = lunchbox::getFilename( filename );
            if( basename == "." || basename == ".." )
                continue;

            // recursively search directories
            const eq::Strings subFiles = lunchbox::searchDirectory( filename,
                                                                    ".*" );
            for(eq::StringsCIter i = subFiles.begin(); i != subFiles.end(); ++i)
                files.push_back( filename + '/' + *i );
        }
    }
}

void Application::_unloadModel()
{
    if( !_modelDist )
        return;

    _modelDist->deregisterTree();
    delete _modelDist;
    _modelDist = 0;

    delete _model;
    _model = 0;
}

const Model* Application::getModel( const eq::uint128_t& modelID )
{
    if( modelID == 0 )
        return 0;
    if( _model )
        return _model;
    lunchbox::memoryBarrier();

    // Accessed concurrently from render threads
    lunchbox::ScopedMutex<> mutex( _modelLock );
    if( _model )
        return _model;

    LBASSERT( !_modelDist );
    _modelDist = new ModelDist;
    Model* model = _modelDist->loadModel( getMasterNode(), this, modelID );
    LBASSERT( model );
    _model = model;

    return model;
}

}
