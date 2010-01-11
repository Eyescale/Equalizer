
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

#ifndef EQBASE_PLUGINREGISTRY_H
#define EQBASE_PLUGINREGISTRY_H

#include "base.h"
#include "compressor.h"
namespace eq 
{
namespace base
{
    /** The registry for all loaded Equalizer plugins. */
    class PluginRegistry
    {
    public:

        /* Search all global plugin directories and register found DSOs */
        void init();
        
        /* Exit all library and free all plugins */
        void exit();
        
        /* @return all registered compressors plugins */
        EQ_EXPORT const CompressorVector& getCompressors() const;

        /* @return the compressor with the given name, or 0. */
        EQ_EXPORT Compressor* findCompressor( const uint32_t name );

    private:
        CompressorVector _compressors;

        /** Initialize a single DSO .*/
        void _initCompressor( const std::string& filename );
    };
}
}
#endif // EQBASE_PLUGINREGISTRY_H
