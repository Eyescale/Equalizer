/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
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

namespace eq
{

bool Compressor::init( const std::string& libraryName )
{

    if( !_dso.open( libraryName ))
        return false;        
    
    getNumCompressors = ( GetNumCompressors_t )
        ( _dso.getFunctionPointer( "EqCompressorGetNumCompressors" ));
    
    CompressorGetInfo_t getInfo = ( CompressorGetInfo_t )
        ( _dso.getFunctionPointer( "EqCompressorGetInfo" ));
    
    compress = ( CompressFunc_t)
        ( _dso.getFunctionPointer( "EqCompressorCompress" ));
       
    decompress = ( DecompressFunc_t)
        ( _dso.getFunctionPointer( "EqCompressorDecompress" ));

    getNumResults = ( GetNumResultsFunc_t)
        ( _dso.getFunctionPointer( "EqCompressorGetNumResults" ));
    
    getResult = ( GetResultFunc_t)
        ( _dso.getFunctionPointer( "EqCompressorGetResult" ));
    
    deleteDecompressor = ( DeleteDecompressor_t )
        ( _dso.getFunctionPointer( "EqCompressorDeleteDecompressor" ));
    
    deleteCompressor = ( DeleteCompressor_t)
        ( _dso.getFunctionPointer( "EqCompressorDeleteCompressor" ));
    
    newCompressor = ( NewCompressor_t)
        ( _dso.getFunctionPointer( "EqCompressorNewCompressor" ));
    
    newDecompressor = ( NewDecompressor_t )
        ( _dso.getFunctionPointer( "EqCompressorNewDecompressor" ));


    if (!( newDecompressor && newCompressor && deleteCompressor && 
          deleteDecompressor && getResult && getNumResults && 
          decompress && compress && getInfo && getNumCompressors ))
       return false;

    const size_t nCompressors = getNumCompressors();
    _infos.resize( nCompressors );

    for( size_t i = 0; i < nCompressors; ++i )
        getInfo( i, &_infos[i] );
    
    return true;
}

bool Compressor::implementsType( const uint32_t name )
{
    for( std::vector<EqCompressorInfo>::const_iterator i = _infos.begin(); 
         i != _infos.end(); ++i )
    {
        if ( i->type == name )
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
}
