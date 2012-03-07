
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010-2012, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef COBASE_COMPRESSOR_H
#define COBASE_COMPRESSOR_H

#include <co/base/thread.h>         // thread-safety macros
#include <co/base/types.h>
#include <co/plugins/compressor.h>  // used inline

namespace co
{
namespace base
{
    /** @internal A C++ class to handle one (de)compressor instance. */
    class Compressor
    {
    public:

        /** Construct a new compressor. */
        COBASE_API Compressor();

        /** Destruct the compressor. */
        COBASE_API virtual ~Compressor();

        /**
         * Initialize the specified compressor or downloader.
         *
         * @param name the name of the compressor
         * @return true on success, false otherwise.
         */
        COBASE_API bool initCompressor( uint32_t name );

        /**
         * Initialize the specified decompressor or uploader. 
         *
         * @param name the name of the compressor
         * @return true on success, false otherwise.
         */
        COBASE_API bool initDecompressor( uint32_t name );

        /** @return the plugin for the current compressor. */
        Plugin* getPlugin() { return _plugin; }
        
        /** @return the name of the compressor. */
        uint32_t getName() const { return _name; }

        /** @return true if the compressor is ready for the 
         *          current compressor name. */
        virtual COBASE_API bool isValid( uint32_t name ) const;

        /** Remove all information about the current compressor. */
        COBASE_API void reset();

        /** @return the quality produced by the current compressor instance. */
        COBASE_API float getQuality() const;

        /** @return the information about the current compressor instance. */
        const CompressorInfo& getInfo() const
            { EQASSERT( _info ); return *_info; }

    protected:
        /** The name of the (de)compressor */
        uint32_t _name;    

        /** Plugin handling the allocation */
        Plugin* _plugin;  
        
        /** The instance of the (de)compressor, can be 0 for decompressor */
        void* _instance;

        /** Info about the current compressor instance */
        const CompressorInfo* _info;

        /** Instance allocation state */
        enum State
        {
            STATE_FREE,
            STATE_COMPRESSOR,
            STATE_DECOMPRESSOR
        } _state;

        /**
         * Find the plugin where located the compressor
         *
         * @param name the name of the compressor 
         */
        Plugin* _findPlugin( uint32_t name );

        EQ_TS_VAR( _thread );
    };
}
}
#endif  // COBASE_COMPRESSOR_H
