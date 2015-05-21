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

#ifndef EQ_FILE_FRAME_WRITER_H
#define EQ_FILE_FRAME_WRITER_H

#include <eq/client/resultImageListener.h> // base class
#include <eq/client/types.h>

namespace eq
{
namespace detail
{

/**
 * Persist the color buffer of a channel to a file.
 * The name of the file is Channel::SATTR_DUMP_IMAGE.rgb
 */
class FileFrameWriter : public ResultImageListener
{
public:
    FileFrameWriter();
    ~FileFrameWriter();

    void notifyNewImage( eq::Channel& channel, const eq::Image& image ) final;
};

}
}

#endif
