
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

#ifndef EQBASE_PLUGINREGISTRY_H
#define EQBASE_PLUGINREGISTRY_H

#include "base.h"
#include "compressor.h"
namespace eq 
{
namespace base
{
    /** The registry for all loaded Equalizer plugins. @internal */
    class PluginRegistry
    {
    public:

        /** Search all global plugin directories and register found DSOs */
        void init();
        
        /** Exit all library and free all plugins */
        void exit();
        
        /** @return all registered compressor plugins */
        EQ_EXPORT const Compressors& getCompressors() const;

        /** @return the plugin containing the given compressor, or 0. */
        EQ_EXPORT Compressor* findCompressor( const uint32_t name );

        /**
         * Find the best compressor in all plugins for the given parameters.
         *
         * This convenience method searches all compressors in all plugins to
         * find the compressor which matches best the given parameters.
         *
         * @param tokenType the structure of the data to compress.
         * @param minQuality minimal quality of the compressed data, with 0 = no
         *                   quality and 1 = full quality, no loss.
         * @param ignoreMSE the most-significant element of each token can be
         *                  ignored, typically the alpha channel of an image.
         */
        EQ_EXPORT uint32_t chooseCompressor( const uint32_t tokenType, 
                                             const float minQuality = 1.0f,
                                             const bool ignoreMSE = false )
            const;

    private:
        Compressors _compressors;

        /** Initialize a single DSO .*/
        void _initCompressor( const std::string& filename );
    };
}
}
#endif // EQBASE_PLUGINREGISTRY_H
