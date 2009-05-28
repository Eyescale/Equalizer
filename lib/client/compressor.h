
/*
 * Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef EQ_COMPRESSOR_H
#define EQ_COMPRESSOR_H
#include <eq/eq.h>
#include <eq/base/dso.h>
#include <eq/plugin/compressor.h>

/**
 * @file client/compressor.h
 * 
 * Helper class for storing the function pointers of one compressor plugin.
 */
namespace eq
{
    typedef void*  ( *NewCompressor_t )     ( const unsigned );
    
    typedef void   ( *DeleteCompressor_t )  
              ( void* const );
              
    typedef void*  ( *NewDecompressor_t )   ( const unsigned );
    
    typedef void   ( *DeleteDecompressor_t )
              ( const unsigned, void* const );
              
    typedef void   ( *CompressFunc_t)       
              ( void* const, void* const, const uint64_t*,
                                    const uint64_t );
    typedef size_t ( *GetNumResultsFunc_t ) (  void* const );
              
    typedef void   ( *GetResultFunc_t )
              ( void* const, const uint32_t, 
                void** const, uint64_t* const );
                
    typedef void   ( *DecompressFunc_t )
              ( void* const, const void** const, 
                const uint64_t* const, const uint32_t, void* const, 
                uint64_t* const, const uint64_t );
                
    typedef void  (*CompressorGetInfo_t) 
             ( const size_t, EqCompressorInfo* const );
                                      
    typedef std::vector< EqCompressorInfo >  CompressorInfoVector;
  
    /**
     * The mirror class for use easily all function of a dso compressor.
     */
  class Compressor
  {
  public:
      typedef size_t ( *GetNumCompressors_t ) ( );

      Compressor(){}

      /* init and link a plugin compressor */
      bool init( const std::string& libraryName );
      
      /* unlink and free all memory pointer */
      void exit();

      /* Get a new compressor instance  */
      NewCompressor_t newCompressor;
        
      /* Get a new decompressor instance  */
      NewDecompressor_t    newDecompressor;
       
      /* delete the compressor instance parametre  */     
      DeleteCompressor_t   deleteCompressor;
        
      /* delete the decompressor instance parametre  */ 
      DeleteDecompressor_t deleteDecompressor;
      
      /* compress data */
      CompressFunc_t       compress;

      /* decompress data */
      DecompressFunc_t     decompress;
      
      /* get the number array that found the results of compressing  */
      GetNumResultsFunc_t  getNumResults;

      /* get the number compressor found in the plugin  */
      GetNumCompressors_t  getNumCompressors;

      /* get the number compressor found in the plugin  */
      GetResultFunc_t   getResult;
      
      /* return true if name is found in the DSO compressor */
      bool implementsType( const uint32_t name );

  private:
      CompressorInfoVector _infos;
      base::DSO _dso;
   
  };

}

#endif //EQ_COMPRESSOR_H
