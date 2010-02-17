
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@equalizergraphics.com>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "initData.h"

InitData::~InitData()
{
    setFrameDataID( EQ_ID_INVALID );
}

void InitData::setFrameDataID( const uint32_t id )
{
    _frameDataID = id;
}

uint32_t InitData::getFrameDataID() const
{
    return _frameDataID;
}

void InitData::getInstanceData( eq::net::DataOStream& stream )
{
    stream << _frameDataID << _modelFileName << _imageFileName;
}

void InitData::applyInstanceData( eq::net::DataIStream& stream )
{
    stream >> _frameDataID >> _modelFileName >> _imageFileName;
    EQASSERT( _frameDataID != EQ_ID_INVALID );
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
    std::string model = _parseCommandLinePrefix( argv, argc, "--model" );
    if( model.size() > 0 )
    {
        setModelFileName( model );
        return true;
    }

    std::string image = _parseCommandLinePrefix( argv, argc, "--image" );
    if( image.size() > 0 )
        setImageFileName( image );

    return true;
}

std::string InitData::_parseCommandLinePrefix( char** argv, int argc,
					       std::string prefix )
{
    for ( int i = 1; i < argc; i++ )
    {
        if( strcmp( argv[i], prefix.c_str( )) == 0 )
        {
	    ++i;
            if( i < argc )
                return argv[i];
        }
    }

    return "";
}
