/* Copyright (c) 2013-2015, Julio Delgado Mangas <julio.delgadomangas@epfl.ch>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <eq/channel.h>
#include <eq/image.h>

#include <lunchbox/log.h>

namespace eq
{
namespace detail
{
FileFrameWriter::FileFrameWriter()
    : ResultImageListener()
{
}

void FileFrameWriter::notifyNewImage( eq::Channel& channel,
                                      const eq::Image& image )
{
    const std::string& prefix =
            channel.getSAttribute( eq::Channel::SATTR_DUMP_IMAGE );
    LBASSERT( !prefix.empty( ));
    const std::string fileName = prefix + channel.getDumpImageFileName();
    if( !image.writeImage( fileName, eq::Frame::BUFFER_COLOR ))
        LBWARN << "Could not write file " << fileName << std::endl;
}

FileFrameWriter::~FileFrameWriter()
{
}

}
}
