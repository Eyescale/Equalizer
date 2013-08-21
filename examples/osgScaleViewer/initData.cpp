
/* Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010-2013, Stefan Eilemann <eile@eyescale.ch>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#include "initData.h"

namespace osgScaleViewer
{

void InitData::setFrameDataID( const eq::UUID& id )
{
    _frameDataID = id;
}

const eq::UUID& InitData::getFrameDataID() const
{
    return _frameDataID;
}

void InitData::getInstanceData( co::DataOStream& stream )
{
    stream << _frameDataID << _modelFileName << _imageFileName;
}

void InitData::applyInstanceData( co::DataIStream& stream )
{
    stream >> _frameDataID >> _modelFileName >> _imageFileName;
}

void InitData::setModelFileName( const std::string &fileName )
{
    _modelFileName = fileName;
}

std::string InitData::getModelFileName() const
{
    return _modelFileName;
}

void InitData::setImageFileName( const std::string& fileName )
{
    _imageFileName = fileName;
}

std::string InitData::getImageFileName() const
{
    return _imageFileName;
}

const std::string InitData::getTrackerPort() const
{
    return _trackerPort;
}

bool InitData::parseCommandLine( char **argv, int argc )
{
    std::string model = _parseCommandLineParam( argc, argv, "--model" );
    if( model.size() > 0 )
    {
        setModelFileName( model );
        return true;
    }

    std::string image = _parseCommandLineParam( argc, argv, "--image" );
    if( image.size() > 0 )
        setImageFileName( image );

    return true;
}

std::string InitData::_parseCommandLineParam( int argc, char** argv,
                                              std::string param )
{
    for ( int i = 1; i < argc; i++ )
    {
        if( strcmp( argv[i], param.c_str( )) == 0 )
        {
            ++i;
            if( i < argc )
                return argv[i];
        }
    }

    return "";
}

}
