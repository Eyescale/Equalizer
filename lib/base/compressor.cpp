
/* Copyright (c) 2009-2010, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "compressor.h"
#include "debug.h"

namespace eq
{
namespace base
{
bool Compressor::init( const std::string& libraryName )
{
    if( !_dso.open( libraryName ))
        return false;        
    
    getNumCompressors = ( GetNumCompressors_t )
        ( _dso.getFunctionPointer( "EqCompressorGetNumCompressors" ));
    
    GetInfo_t getInfo = ( GetInfo_t )
        ( _dso.getFunctionPointer( "EqCompressorGetInfo" ));
    
    compress = ( Compress_t )
        ( _dso.getFunctionPointer( "EqCompressorCompress" ));
       
    decompress = ( Decompress_t )
        ( _dso.getFunctionPointer( "EqCompressorDecompress" ));

    getNumResults = ( GetNumResults_t )
        ( _dso.getFunctionPointer( "EqCompressorGetNumResults" ));
    
    getResult = ( GetResult_t )
        ( _dso.getFunctionPointer( "EqCompressorGetResult" ));
    
    deleteDecompressor = ( DeleteDecompressor_t )
        ( _dso.getFunctionPointer( "EqCompressorDeleteDecompressor" ));
    
    deleteCompressor = ( DeleteCompressor_t)
        ( _dso.getFunctionPointer( "EqCompressorDeleteCompressor" ));
    
    newCompressor = ( NewCompressor_t)
        ( _dso.getFunctionPointer( "EqCompressorNewCompressor" ));
    
    newDecompressor = ( NewDecompressor_t )
        ( _dso.getFunctionPointer( "EqCompressorNewDecompressor" ));

    isCompatible = ( IsCompatible_t )
        ( _dso.getFunctionPointer( "EqCompressorIsCompatible" ));
    
    download = ( Download_t )
        ( _dso.getFunctionPointer( "EqCompressorDownload" ));
    
    upload = ( Upload_t )
        ( _dso.getFunctionPointer( "EqCompressorUpload" ));
    
    const bool foundBase = newDecompressor && newCompressor &&
        deleteCompressor && deleteDecompressor && getInfo && getNumCompressors;
    const bool foundCPU = getResult && getNumResults && decompress && compress;
    const bool foundGPU = isCompatible && download && upload;

    if( !foundBase || ( !foundCPU && !foundGPU ))
    {
        EQWARN << "Initializing compression DSO " << libraryName 
           << " failed, at least one entry point missing" << std::endl;
        return false;
    }

    const size_t nCompressors = getNumCompressors();
    EQASSERT( nCompressors > 0 );
    _infos.resize( nCompressors );

    for( size_t i = 0; i < nCompressors; ++i )
    {
        EqCompressorInfo& info = _infos[ i ];

        info.outputTokenType = EQ_COMPRESSOR_DATATYPE_NONE;
        info.outputTokenSize = 0;
        getInfo( i, &info );

        if( !( info.capabilities & EQ_COMPRESSOR_TRANSFER ))
        {
            if( info.outputTokenType == EQ_COMPRESSOR_DATATYPE_NONE )
            {
                // Set up CPU compressor output to be input type
                info.outputTokenType = info.tokenType;
                EQASSERT( info.outputTokenSize == 0 );
            }
            else
            {
                EQASSERT( info.outputTokenSize != 0 );
            }
        }
    }

    return true;
}

bool Compressor::implementsType( const uint32_t name )
{
    for( std::vector<EqCompressorInfo>::const_iterator i = _infos.begin(); 
         i != _infos.end(); ++i )
    {
        if ( i->name == name )
            return true;
    }

    return false;
}

void Compressor::exit()
{
    _dso.close();
    _infos.clear();

    newCompressor = 0;
    newDecompressor = 0;     
    deleteCompressor = 0;
    deleteDecompressor = 0;
    compress = 0;
    decompress = 0;
    getNumResults = 0;
    getNumCompressors = 0;
    getResult = 0;

}

std::ostream& operator << ( std::ostream& os, const EqCompressorInfo& info )
{
    os << "v" << info.version << " name " << info.name << " token "
       << info.tokenType << " cap " << info.capabilities << " quality "
       << info.quality << " ratio " << info.ratio << " speed " << info.speed;
    return os;
}

}
}
