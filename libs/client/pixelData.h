
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

#ifndef EQ_PIXELDATA_H
#define EQ_PIXELDATA_H

#include <eq/fabric/pixelViewport.h> // member
#include <eq/fabric/viewport.h>      // member
#include <eq/util/texture.h>         // member
#include <eq/util/types.h>
#include <co/base/buffer.h>          // member

#include <co/plugins/compressor.h> // EqCompressorInfos typedef

namespace eq
{
    /** The pixel data structure manages the pixel information for images. */
    struct PixelData : public co::base::NonCopyable
    {
        /** Construct new pixel data. @version 1.0 */
        EQ_API PixelData();

        /** Destruct the pixel data. @version 1.0 */
        EQ_API ~PixelData();

        /**
         * Reset the data.
         *
         * This will not free the data pointed to by pixels and
         * compressedPixels.
         * @version 1.0
         */
        void reset();

        /**
         * The type of data stored in FrameBuffer or texture on the GPU.
         * @sa the input token types in plugins/compressor.h
         * @version 1.0
         */
        uint32_t internalFormat;

        /**
         * The type of data stored in pixels in main memory.
         * @sa the output token types in plugins/compressor.h
         * @version 1.0
         */
        uint32_t externalFormat;

        /**
         * The size of one pixel, in bytes, stored in pixels.
         * @sa the output token types in plugins/compressor.h
         * @version 1.0
         */
        uint32_t pixelSize;

        /**
         * The dimensions of the pixel data in pixels.
         *
         * Note that this pvp might differ from the image pvp since the data is
         * downloaded from the GPU using a plugin, which might compress the
         * data. If unmodified pixel data is required, the correct download
         * plugin has to be used.
         *
         * @sa Image::allocDownloader()
         * @version 1.0
         */
        PixelViewport pvp;

        /** uncompressed pixel data, pvp * pixelSize bytes. @version 1.0 */
        void* pixels;

        /** The compressed pixel data blocks. @version 1.0 */
        std::vector< void* > compressedData;

        /** Sizes of each compressed pixel data block. @version 1.0 */
        std::vector< uint64_t > compressedSize;

        /** The compressor used to produce compressedData. @version 1.0 */
        uint32_t compressorName;
        uint32_t compressorFlags; //!< Flags used for compression. @version 1.0
        bool isCompressed; //!< The compressed pixel data is set. @version 1.0
    };
};
#endif // EQ_PIXELDATA_H
