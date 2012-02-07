
/* Copyright (c)      2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "gpuCompressor.h"

#include <eq/client/gl.h>
#include <eq/fabric/pixelViewport.h>

#include <co/base/compressorInfo.h>
#include <co/base/global.h>
#include <co/base/plugin.h>
#include <co/base/pluginRegistry.h>

namespace eq
{
namespace util
{

bool GPUCompressor::isValidDownloader( const uint32_t internalFormat,
                                       const bool ignoreAlpha,
                                       const uint64_t capabilities ) const
{
    return (_plugin && isValid( _name ) && _info->tokenType == internalFormat &&
             (( _info->capabilities & capabilities ) == capabilities ) &&
            ( ignoreAlpha || hasAlpha( )));
}

bool GPUCompressor::isValidUploader( const uint32_t externalFormat, 
                                     const uint32_t internalFormat,
                                     const uint64_t capabilities ) const
{
    return _plugin && _info->outputTokenType == externalFormat &&
           (( _info->capabilities & capabilities ) == capabilities ) &&
           _info->tokenType == internalFormat;
}

bool GPUCompressor::initDownloader( const uint32_t internalFormat,
                                    const float    minQuality,
                                    const bool     ignoreAlpha, 
                                    const uint64_t capabilities )
{
    EQASSERT( _glewContext );
    float ratio = std::numeric_limits< float >::max();
    float speed = 0;
    uint32_t name = EQ_COMPRESSOR_NONE;
    
    co::base::CompressorInfos infos;
    findTransferers( internalFormat, 0, capabilities, minQuality, 
                     ignoreAlpha, _glewContext, infos );
    
    for( co::base::CompressorInfos::const_iterator i = infos.begin();
         i != infos.end(); ++i )
    {
        const co::base::CompressorInfo& info = *i;

        if( ratio > info.ratio || 
            ( ratio == info.ratio && speed < info.speed))
        {
            ratio = info.ratio;
            speed = info.speed;
            name = info.name;
        }
    }

    if( name == EQ_COMPRESSOR_NONE )
    {
        reset();
        return false;
    }
    if( name != _name )
        return initCompressor( name );
    return true;
}

bool GPUCompressor::initDownloader( const uint32_t name )
{
    EQASSERT( name > EQ_COMPRESSOR_NONE );
    return initCompressor( name );
}

bool GPUCompressor::initUploader( const uint32_t externalFormat,
                                  const uint32_t internalFormat,
                                  const uint64_t capabilities )
{
    co::base::PluginRegistry& registry = co::base::Global::getPluginRegistry();
    const co::base::Plugins& plugins = registry.getPlugins();

    uint32_t name = EQ_COMPRESSOR_NONE;
    float speed = 0.0f;
    for( co::base::Plugins::const_iterator i = plugins.begin();
         i != plugins.end(); ++i )
    {
        const co::base::Plugin* plugin = *i;
        const co::base::CompressorInfos& infos = plugin->getInfos();

        for( co::base::CompressorInfos::const_iterator j = infos.begin();
             j != infos.end(); ++j )
        {
            const co::base::CompressorInfo& info = *j;
            
            if( (info.capabilities & capabilities) != capabilities ||
                info.outputTokenType != externalFormat ||
                info.tokenType != internalFormat ||
                !plugin->isCompatible( info.name, _glewContext ))
            {
                continue;
            }

            if( speed < info.speed )
            {
                speed = info.speed;
                name = info.name;
                _info = &info;
            }
        }
    }

    EQASSERT( name != EQ_COMPRESSOR_NONE );
    if( name == EQ_COMPRESSOR_NONE )
    {
        reset();
        return false;
    }
    if( name != _name )
        return initDecompressor( name );
    return true;
}

bool GPUCompressor::startDownload( const fabric::PixelViewport& pvpIn,
                                   const unsigned source, const uint64_t flags,
                                   fabric::PixelViewport& pvpOut, void** out )
{
    EQASSERT( _plugin );
    EQASSERT( _glewContext );

    const uint64_t inDims[4] = { pvpIn.x, pvpIn.w, pvpIn.y, pvpIn.h }; 

    if( _info->capabilities & EQ_COMPRESSOR_USE_ASYNC_DOWNLOAD )
    {
        _plugin->startDownload( _instance, _name, _glewContext, inDims, source,
                                flags | EQ_COMPRESSOR_USE_ASYNC_DOWNLOAD );
        return true;
    }

    uint64_t outDims[4] = { 0, 0, 0, 0 };
    _plugin->download( _instance, _name, _glewContext, inDims, source, flags,
                       outDims, out );
    pvpOut.x = outDims[0];
    pvpOut.w = outDims[1];
    pvpOut.y = outDims[2];
    pvpOut.h = outDims[3];
    return false;
}


void GPUCompressor::finishDownload( const fabric::PixelViewport& pvpIn,
                                    const uint64_t flags,
                                    fabric::PixelViewport& pvpOut, void** out )
{
    EQASSERT( _plugin );
    EQASSERT( _glewContext );

    if( _info->capabilities & EQ_COMPRESSOR_USE_ASYNC_DOWNLOAD )
    {
        const uint64_t inDims[4] = { pvpIn.x, pvpIn.w, pvpIn.y, pvpIn.h }; 
        uint64_t outDims[4] = { 0, 0, 0, 0 };
        _plugin->finishDownload( _instance, _name, _glewContext, inDims,
                                 flags | EQ_COMPRESSOR_USE_ASYNC_DOWNLOAD,
                                 outDims, out );
        pvpOut.x = outDims[0];
        pvpOut.w = outDims[1];
        pvpOut.y = outDims[2];
        pvpOut.h = outDims[3];
    }
}

void GPUCompressor::upload( const void*                  buffer,
                            const fabric::PixelViewport& pvpIn,
                            const uint64_t               flags,
                            const fabric::PixelViewport& pvpOut,  
                            const unsigned               destination )
{
    EQASSERT( _plugin );
    EQASSERT( _glewContext );

    const uint64_t inDims[4] = { pvpIn.x, pvpIn.w, pvpIn.y, pvpIn.h }; 
    uint64_t outDims[4] = { pvpOut.x, pvpOut.w, pvpOut.y, pvpOut.h };
    _plugin->upload( _instance, _name, _glewContext, buffer, inDims,
                     flags, outDims, destination );
}

uint32_t GPUCompressor::getExternalFormat() const
{
    return _info->outputTokenType;
}

uint32_t GPUCompressor::getInternalFormat() const
{
    return _info->tokenType;
}

uint32_t GPUCompressor::getTokenSize() const
{
    return _info->outputTokenSize;
}

bool GPUCompressor::hasAlpha() const
{
    return (_info->capabilities & EQ_COMPRESSOR_IGNORE_ALPHA) == 0;
}

uint32_t GPUCompressor::getExternalFormat( const uint32_t format,
                                           const uint32_t type )
{
    switch( format )
    {
        case GL_BGRA:
            switch ( type )
            {
                case GL_UNSIGNED_INT_8_8_8_8_REV : 
                    return EQ_COMPRESSOR_DATATYPE_BGRA_UINT_8_8_8_8_REV;
                case GL_UNSIGNED_BYTE : 
                    return EQ_COMPRESSOR_DATATYPE_BGRA;
                case GL_HALF_FLOAT : 
                    return EQ_COMPRESSOR_DATATYPE_BGRA16F;
                case GL_FLOAT : 
                    return EQ_COMPRESSOR_DATATYPE_BGRA32F;
                case GL_UNSIGNED_INT_10_10_10_2 : 
                    return EQ_COMPRESSOR_DATATYPE_BGR10_A2;
            }
            break;

        case GL_RGBA:
            switch ( type )
            {
                case GL_UNSIGNED_INT_8_8_8_8_REV : 
                    return EQ_COMPRESSOR_DATATYPE_RGBA_UINT_8_8_8_8_REV;
                case GL_UNSIGNED_BYTE : 
                    return EQ_COMPRESSOR_DATATYPE_RGBA;
                case GL_HALF_FLOAT : 
                    return EQ_COMPRESSOR_DATATYPE_RGBA16F;
                case GL_FLOAT : 
                    return EQ_COMPRESSOR_DATATYPE_RGBA32F;
            }
            break;
    
        case GL_RGB:
            switch ( type )
            {
                case GL_UNSIGNED_BYTE : 
                    return EQ_COMPRESSOR_DATATYPE_RGB;
                case GL_HALF_FLOAT : 
                    return EQ_COMPRESSOR_DATATYPE_RGB16F;
                case GL_FLOAT :
                    return EQ_COMPRESSOR_DATATYPE_RGB32F;   
            }
            break;

        case GL_BGR:
            switch ( type )
            {
                case GL_UNSIGNED_BYTE : 
                    return EQ_COMPRESSOR_DATATYPE_BGR;
                case GL_HALF_FLOAT : 
                    return EQ_COMPRESSOR_DATATYPE_BGR16F;
                case GL_FLOAT : 
                    return EQ_COMPRESSOR_DATATYPE_BGR32F;
            }

        case GL_DEPTH_COMPONENT:
            switch ( type )
            {
                case GL_UNSIGNED_INT : 
                    return EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT;
                case GL_FLOAT : 
                    return EQ_COMPRESSOR_DATATYPE_DEPTH_FLOAT;
            }
    }

    EQASSERTINFO( false, "Not implemented" );
    return 0;
}

void GPUCompressor::findTransferers( const uint32_t internalFormat,
                                     const uint32_t externalFormat,
                                     const uint64_t capabilities,
                                     const float    minQuality,
                                     const bool     ignoreAlpha,
                                     const GLEWContext* glewContext,
                                     co::base::CompressorInfos& result )
{
    const co::base::PluginRegistry& registry =
                                        co::base::Global::getPluginRegistry();
    const co::base::Plugins& plugins = registry.getPlugins();
    const uint64_t caps = capabilities | EQ_COMPRESSOR_TRANSFER;

    for( co::base::Plugins::const_iterator i = plugins.begin();
         i != plugins.end(); ++i )
    {
        const co::base::Plugin* plugin = *i;
        const co::base::CompressorInfos& infos = plugin->getInfos();
        for( co::base::CompressorInfos::const_iterator j = infos.begin();
             j != infos.end(); ++j )
        {
            const co::base::CompressorInfo& info = *j;
            if(( (info.capabilities & caps) == caps )  &&
               ( internalFormat == EQ_COMPRESSOR_DATATYPE_NONE ||
                 info.tokenType == internalFormat )                    &&
               ( externalFormat == EQ_COMPRESSOR_DATATYPE_NONE ||
                 info.outputTokenType == externalFormat )              &&
               ( info.quality >= minQuality )                          &&
               ( ignoreAlpha ||
                 !(info.capabilities & EQ_COMPRESSOR_IGNORE_ALPHA ))   &&
               ( !glewContext || plugin->isCompatible( info.name, glewContext)))
            {
                result.push_back( info );
            }
        }
    }
}

}
}
