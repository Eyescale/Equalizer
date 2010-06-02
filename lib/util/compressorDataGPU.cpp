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

#include "compressorDataGPU.h"
#include "eq/base/global.h"
#include "eq/base/pluginRegistry.h"
#include "GL/glew.h"

namespace eq
{
namespace util
{

bool CompressorDataGPU::isValidDownloader( uint32_t inputToken )
{
    return isValid( _name ) && _info.tokenType == inputToken ;
}

bool CompressorDataGPU::isValidUploader( uint32_t inputToken, 
                                         uint32_t outputToken )
{
    return ( _info.outputTokenType == inputToken ) &&
           ( _info.tokenType == outputToken );
}

void CompressorDataGPU::download( const fabric::PixelViewport& pvpIn,
                                  const unsigned     source,
                                  const eq_uint64_t  flags,
                                  fabric::PixelViewport&       pvpOut,
                                  void**             out )
{
    const uint64_t inDims[4] = { pvpIn.x, pvpIn.w, pvpIn.y, pvpIn.h }; 
    uint64_t outDims[4] = { 0, 0, 0, 0 };
    _plugin->download( _instance, _name, _glewContext,
                       inDims, source, flags, outDims, out );
    pvpOut.x = outDims[0];
    pvpOut.w = outDims[1];
    pvpOut.y = outDims[2];
    pvpOut.h = outDims[3];
}

void CompressorDataGPU::upload( const void*          buffer,
                                const fabric::PixelViewport& pvpIn,
                                const eq_uint64_t    flags,
                                const fabric::PixelViewport& pvpOut,  
                                const unsigned       destination )
{
    const uint64_t inDims[4] = { pvpIn.x, pvpIn.w, pvpIn.y, pvpIn.h }; 
    uint64_t outDims[4] = { pvpOut.x, pvpOut.w, pvpOut.y, pvpOut.h };
    _plugin->upload( _instance, _name, _glewContext, buffer, inDims,
                     flags, outDims, destination );
}

void CompressorDataGPU::initUploader( uint32_t inTokenType, 
                                      uint32_t outTokenType )
{
    base::PluginRegistry& registry = base::Global::getPluginRegistry();
    const base::Compressors& compressors = registry.getCompressors();

    uint32_t name = EQ_COMPRESSOR_NONE;
    float speed = 0.0f;
    for( base::Compressors::const_iterator i = compressors.begin();
         i != compressors.end(); ++i )
    {
        const base::Compressor* compressor = *i;
        const base::CompressorInfos& infos = compressor->getInfos();

        EQINFO << "Searching in DSO " << (void*)compressor << std::endl;
        
        for( base::CompressorInfos::const_iterator j = infos.begin();
             j != infos.end(); ++j )
        {
            const EqCompressorInfo& info = *j;
            
            if(( info.capabilities & EQ_COMPRESSOR_TRANSFER ) == 0 )
                continue;

            if( info.outputTokenType != inTokenType )
                continue;

            if( info.tokenType != outTokenType )
                continue;
            
            if( !compressor->isCompatible( info.name, _glewContext ))
                continue;
            
            if ( speed < info.speed )
            {
                speed = info.speed;
                name = info.name;
                _info = info;
            }
        }
    }

    if ( name == EQ_COMPRESSOR_NONE )
        reset();
    else if( name != _name )
        _initCompressor( name );
}

void CompressorDataGPU::initDownloader( float minQuality, 
                                        uint32_t tokenType )
{ 
    float factor = 1.1f;
    uint32_t name = EQ_COMPRESSOR_NONE;
    
    base::CompressorInfos infos; 
    addTransfererInfos( infos, minQuality, tokenType, _glewContext );
    
    for( base::CompressorInfos::const_iterator j = infos.begin();
         j != infos.end(); ++j )
    {
        const EqCompressorInfo& info = *j;
        
        if ( factor > ( info.ratio * info.quality ))
        {
            factor = ( info.ratio * info.quality );
            name = info.name;
            _info = info;
        }
    }

    if ( name == EQ_COMPRESSOR_NONE )
        reset();
    else if( name != _name )
    {
        _initCompressor( name );
    }
}

bool CompressorDataGPU::initDownloader( uint64_t name )
{
    EQASSERT( EQ_COMPRESSOR_NONE );
    if( name != _name )
    {
        _initCompressor( name );
    }
    return true;
}
uint32_t CompressorDataGPU::getPixelSize( uint64_t pixelType )
{
    switch( pixelType )
    {
        case EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_BYTE:
        case EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_INT_8_8_8_8_REV:
        case EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_BYTE:
        case EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_INT_8_8_8_8_REV:
        case EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_INT_10_10_10_2:
        case EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_INT_10_10_10_2:
            return 4;
        case EQ_COMPRESSOR_DATATYPE_RGB_UNSIGNED_BYTE:
        case EQ_COMPRESSOR_DATATYPE_BGR_UNSIGNED_BYTE:
            return 3;
        case EQ_COMPRESSOR_DATATYPE_RGBA_FLOAT:
        case EQ_COMPRESSOR_DATATYPE_BGRA_FLOAT:
            return 16;
        case EQ_COMPRESSOR_DATATYPE_RGB_FLOAT:
        case EQ_COMPRESSOR_DATATYPE_BGR_FLOAT:
            return 12;
        case EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT:
        case EQ_COMPRESSOR_DATATYPE_DEPTH_FLOAT:
        case EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT_24_8_NV:
            return 4;
        case EQ_COMPRESSOR_DATATYPE_RGBA_HALF_FLOAT:
        case EQ_COMPRESSOR_DATATYPE_BGRA_HALF_FLOAT:
            return 8;
        case EQ_COMPRESSOR_DATATYPE_RGB_HALF_FLOAT:
        case EQ_COMPRESSOR_DATATYPE_BGR_HALF_FLOAT:
            return 6;

        default:
            EQERROR << "Unknown image pixel data type" << std::endl;
            return 0;
    }
}

uint32_t CompressorDataGPU::getTokenFormat( uint32_t format,
                                            uint32_t type )
{
    if( format == GL_BGRA )
    {
        switch ( type )
        {
            case GL_UNSIGNED_INT_8_8_8_8_REV : 
                return EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_INT_8_8_8_8_REV;
            case GL_UNSIGNED_BYTE : 
                return EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_BYTE;
            case GL_HALF_FLOAT : 
                return EQ_COMPRESSOR_DATATYPE_BGRA_HALF_FLOAT;
            case GL_FLOAT : 
                return EQ_COMPRESSOR_DATATYPE_BGRA_FLOAT;
            case GL_UNSIGNED_INT_10_10_10_2 : 
                return EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_INT_10_10_10_2;
        }
    }
    else if( format == GL_RGBA )
    {
        switch ( type )
        {
            case GL_UNSIGNED_INT_8_8_8_8_REV : 
                return EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_INT_8_8_8_8_REV;
            case GL_UNSIGNED_BYTE : 
                return EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_BYTE;
            case GL_HALF_FLOAT : 
                return EQ_COMPRESSOR_DATATYPE_RGBA_HALF_FLOAT;
            case GL_FLOAT : 
                return EQ_COMPRESSOR_DATATYPE_RGBA_FLOAT;
            case GL_UNSIGNED_INT_10_10_10_2 : 
                return EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_INT_10_10_10_2;
        }
    }
    else if( format == GL_RGB )
    {
        switch ( type )
        {
            case GL_UNSIGNED_BYTE : 
                return EQ_COMPRESSOR_DATATYPE_RGB_UNSIGNED_BYTE;
            case GL_HALF_FLOAT : 
                return EQ_COMPRESSOR_DATATYPE_RGB_HALF_FLOAT;
            case GL_FLOAT : 
                return EQ_COMPRESSOR_DATATYPE_RGB_FLOAT;   
        }
    }
    else if( format == GL_BGR )
    {
        switch ( type )
        {
            case GL_UNSIGNED_BYTE : 
                return EQ_COMPRESSOR_DATATYPE_BGR_UNSIGNED_BYTE;
            case GL_HALF_FLOAT : 
                return EQ_COMPRESSOR_DATATYPE_BGR_HALF_FLOAT;
            case GL_FLOAT : 
                return EQ_COMPRESSOR_DATATYPE_BGR_FLOAT;
        }
    }
    else if ( format == GL_DEPTH_COMPONENT )
    {
        switch ( type )
        {
            case GL_UNSIGNED_INT : 
                return EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT;
            case GL_FLOAT : 
                return EQ_COMPRESSOR_DATATYPE_DEPTH_FLOAT;
        }
    }
    else if ( format == GL_DEPTH_STENCIL_NV )
    {
        return EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT_24_8_NV;
    }
    EQASSERT( false );
    return 0;
}

void CompressorDataGPU::addTransfererInfos( eq::base::CompressorInfos& outInfos,
                                            float minQuality, 
                                            uint32_t tokenType,
                                            GLEWContext* glewContext )
{
    const eq::base::PluginRegistry& registry = eq::base::Global::getPluginRegistry();
    const eq::base::Compressors& plugins = registry.getCompressors();

    for( eq::base::Compressors::const_iterator i = plugins.begin();
         i != plugins.end(); ++i )
    {
        const base::Compressor* compressor = *i;
        const eq::base::CompressorInfos& infos = (*i)->getInfos();
        for( eq::base::CompressorInfos::const_iterator j = infos.begin();
             j != infos.end(); ++j )
        {
            const EqCompressorInfo& info = *j;
            
            if(( info.capabilities & EQ_COMPRESSOR_TRANSFER ) == 0 )
                continue;
            
            if( info.tokenType != tokenType )
                continue;

            if( info.quality < minQuality )
                continue;

            if( !compressor->isCompatible( info.name, glewContext ))
                continue;
            
            outInfos.push_back( info );
        }
    }
}

}
}