
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQUTIL_COMPRESSORDATA_H
#define EQUTIL_COMPRESSORDATA_H

#include <eq/base/buffer.h>
#include <eq/base/compressor.h>

namespace eq
{
namespace base
{
    /** A C++ class to abstract a compressor instance. */
    class CompressorData
    {
    public:

        /** Construct a new compressor data. */
        EQ_EXPORT CompressorData();

        /** Destruct the compressor data */
        EQ_EXPORT virtual ~CompressorData();

        /** @return the plugin for the current compressor. */
        base::Compressor* getPlugin(){ return _plugin; }
        
        /** @return the name of the compressor. */
        uint32_t getName() const { return _name; }

        /** @return true if the compressor is ready for the 
         *          current compressor name. */
        virtual EQ_EXPORT bool isValid( uint32_t name ) const;

        /** Remove all information about the current compressor. */
        EQ_EXPORT void reset();

        /** @return the quality produced by the current compressor instance. */
        float getQuality() const
            { return _info ? _info->quality : 1.0f; }

        /** @return the information about the current compressor instance. */
        const EqCompressorInfo* getInfo() const
            { EQASSERT( _info ); return _info; }

    protected:
        /** The name of the (de)compressor */
        uint32_t _name;    

        /** Plugin handling the allocation */
        base::Compressor* _plugin;  
        
        /** The instance of the (de)compressor, can be 0 for decompressor */
        void* _instance;

        /** Info about the current compressor instance */
        const EqCompressorInfo* _info;

        /** true if the instance is a compressor, false if downloader */
        bool _isCompressor;

        /**
         * Find the plugin where located the compressor
         *
         * @param name the name of the compressor 
         */
        base::Compressor* _findPlugin( uint32_t name );

        /**
         * Initialize the specified compressor or downloader 
         *
         * @param name the name of the compressor
         */
        bool _initCompressor( uint32_t name );

        /**
         * Initialize the specified decompressor or uploader 
         *
         * @param name the name of the compressor
         */
        bool _initDecompressor( uint32_t name );
    };
}
}
#endif  // EQUTIL_COMPRESSORDATA_H
