
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef EQUTIL_COMPRSSORDATA_H
#define EQUTIL_COMPRSSORDATA_H

#include "buffer.h"
#include "compressor.h"

namespace eq
{
namespace base
{
    /** A C++ class to abstract a compressor instance */
    class CompressorData
    {
    public:

        /** Construct a new compressorData */
        CompressorData( )
                : _name( 0 )
                , _instance( 0 )
                , _plugin( 0 )
                , _isCompressor( true ){}
		
		/** destructor of compressorData */
		virtual ~CompressorData(){}
        
        /** @return the plugin for the current compressor. */
        base::Compressor* getPlugin(){ return _plugin; }
        
        /** @return the name of the compressor. */
        uint32_t getName() const { return _name; }

        /** Set the name of the compressor. */
        void setName( uint32_t name );

        /** @return true if the compressor is ready for the 
                    current compressor name. */
        virtual EQ_EXPORT bool isValid( uint32_t name );

        uint32_t getTypeProcessing() const { return _info.outputTokenType; }
        
        uint32_t getSizeTypeProcessing() const { return _info.outputTokenSize; }
        
        /** remove all information about the current comprossor. */
        EQ_EXPORT void reset();

    protected:
        /** the name of the (de)compressor */
        uint32_t          _name;    
        
        /** the instance of the (de)compressor */
        void*             _instance;
        
        /** Plugin handling the allocation */
        base::Compressor* _plugin;  
        
        EqCompressorInfo  _info;

        float _quality;

        /** If the insttance is a compressor or downloader Type */
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
         * @param isCompressor compressor or decompressor  
         */
        bool _initCompressor( uint32_t name );

        /**
         * Initialize the specified decompressor or uploader 
         *
         * @param name the name of the compressor
         * @param isCompressor compressor or decompressor  
         */
        bool _initDecompressor( uint32_t name );        
    };
}
}
#endif  // EQUTIL_COMPRSSORDATA_H
