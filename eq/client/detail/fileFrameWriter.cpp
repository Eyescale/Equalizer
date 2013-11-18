
/* Copyright (c) 2013, Julio Delgado Mangas <julio.delgadomangas@epfl.ch>
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

#include "fileFrameWriter.h"

#include <eq/client/channel.h>
#include <eq/client/image.h>
#include <eq/client/gl.h>
#include <eq/client/pipe.h>

#include <lunchbox/log.h>

#include <sstream>

namespace
{
std::string _buildFileName( const eq::Channel* channel )
{
    std::stringstream name;
    name << channel->getSAttribute( eq::Channel::SATTR_DUMP_IMAGE )
         << channel->getPipe()->getCurrentFrame( ) << ".rgb";
    return name.str();
}
}

namespace eq
{
namespace detail
{
FileFrameWriter::FileFrameWriter()
{
    _image.setAlphaUsage( true );
    _image.setQuality( eq::Frame::BUFFER_COLOR, 1.0f );
    _image.setStorageType( eq::Frame::TYPE_MEMORY );
    _image.setInternalFormat( eq::Frame::BUFFER_COLOR, GL_RGBA );
}

void FileFrameWriter::write( eq::Channel* channel )
{
    if( _image.readback( eq::Frame::BUFFER_COLOR, channel->getPixelViewport(),
                          channel->getZoom(), channel->getObjectManager( )))
    {
        const std::string& fileName = _buildFileName( channel );
        if( !_image.writeImage( fileName, eq::Frame::BUFFER_COLOR ))
            LBWARN << "Could not write file " << fileName << std::endl;
    }
    else
        LBWARN << "Could not read frame buffer" << std::endl;
}

FileFrameWriter::~FileFrameWriter()
{
	_image.flush();
}

}
}
