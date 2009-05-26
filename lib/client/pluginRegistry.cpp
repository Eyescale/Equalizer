
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
#include "pluginRegistry.h"
#include "compressor.h" 
#include "global.h"


#include <eq/base/fileSearch.h> 
#include <eq/base/dso.h> 
#include <vector>

namespace eq
{


void PluginRegistry::init()
{
   EQASSERT( _compressors.empty( ));

    // search all plugin directories for compressor DSOs
    const StringVector& directories = Global::getPluginDirectories();

    // for each directory
    for( StringVector::const_iterator i = directories.begin();
         i != directories.end(); ++i )
    {
        const std::string& directory = *i;
        // search the number of files in the director<y
#ifdef WIN32
        StringVector files = base::fileSearch( directory, 
                                                "libeqCompressor*.dll" );
        const char DIRSEP = '\\';
#elif defined (Darwin)
        StringVector files = base::fileSearch( directory, 
                                                "libeqCompressor*dylib" );
        const char DIRSEP = '/';
#else
        StringVector files = base::fileSearch( directory,
                                                "libeqCompressor*so" );
        const char DIRSEP = '/';
#endif
        
        // for each file in the directoy
        for( StringVector::const_iterator j = files.begin();
             j != files.end(); ++j )
        {
            // build path + name of library
            const std::string libraryName = directory + DIRSEP + *j;
           
            Compressor* compressor = new Compressor(); 
            if( compressor->init( libraryName ))
                _compressors.push_back( compressor );
            else
                delete compressor;
        }
    }
}


void PluginRegistry::exit()
{
    for( CompressorVector::const_iterator i = _compressors.begin(); 
         i != _compressors.end(); ++i )
    {
        Compressor* compressor = *i;
        compressor->exit();
        delete compressor;
    }
}

Compressor* PluginRegistry::findCompressor( const uint32_t name )
{

    for( CompressorVector::const_iterator i = _compressors.begin(); 
         i != _compressors.end(); ++i )
    {
        Compressor* compressor = *i;
        if ( compressor->implementsType( name ))
            return compressor;
    }

    return 0;
}

const CompressorVector& PluginRegistry::getCompressors() const
{
    return _compressors;
}


}
