
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "pixelData.h"

namespace eq
{
PixelData::PixelData()
{
    reset();
}

PixelData::~PixelData()
{
    reset();
}

void PixelData::reset()
{
    internalFormat = EQ_COMPRESSOR_DATATYPE_NONE;
    externalFormat = EQ_COMPRESSOR_DATATYPE_NONE;
    pixelSize = 0;
    pixels = 0;
    compressorName = EQ_COMPRESSOR_INVALID;
    compressorFlags = 0;
    isCompressed = false;
    compressedSize.clear();
    compressedData.clear();
}

}
