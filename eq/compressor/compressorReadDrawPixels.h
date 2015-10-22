
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010-2014, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQ_PLUGIN_COMPRESSORREADDRAWPIXELS
#define EQ_PLUGIN_COMPRESSORREADDRAWPIXELS

#include "compressor.h"
#include <eq/gl.h>
#include <eq/util/types.h>

namespace eq
{
namespace plugin
{

enum FlushMode
{
    FLUSH_TEXTURE,
    KEEP_TEXTURE
};

class CompressorReadDrawPixels : public Compressor
{
public:
    explicit CompressorReadDrawPixels( const unsigned name );
    virtual ~CompressorReadDrawPixels();

    static void* getNewCompressor( const unsigned name )
        { return new CompressorReadDrawPixels( name ); }

    static void* getNewDecompressor( const unsigned name )
        { return new CompressorReadDrawPixels( name ); }

    void compress( const void* const, const eq_uint64_t, const bool ) override
        { LBDONTCALL; }

    static bool isCompatible( const GLEWContext* );

    void download( const GLEWContext*, const eq_uint64_t*, const unsigned,
                   const eq_uint64_t, eq_uint64_t*, void** ) override;

    void upload( const GLEWContext*, const void*, const eq_uint64_t*,
                 const eq_uint64_t, const eq_uint64_t*,
                 const unsigned ) override;

    void startDownload( const GLEWContext*, const eq_uint64_t*, const unsigned,
                        const eq_uint64_t) override;

    void finishDownload( const GLEWContext*, const eq_uint64_t*,
                         const eq_uint64_t, eq_uint64_t*, void** ) override;

protected:
    lunchbox::Bufferb _buffer;
    util::Texture*    _texture;
    util::PixelBufferObject* _pbo;
    unsigned    _internalFormat; //!< the GL format
    unsigned    _format;         //!< the GL format
    unsigned    _type;           //!< the GL type
    const unsigned _depth;       //!< the size of one output token

    void _resizeBuffer( const eq_uint64_t );
    void _initTexture( const GLEWContext*, const eq_uint64_t );
    void _initAsyncTexture( const GLEWContext*, const eq_uint64_t,
                            const eq_uint64_t );
    bool _initPBO( const GLEWContext*, const eq_uint64_t );
    void _initDownload( const GLEWContext*, const eq_uint64_t*, eq_uint64_t* );
    void* _downloadTexture( const GLEWContext* glewContext,
                            const FlushMode mode );
};

}
}
#endif // EQ_PLUGIN_COMPRESSORREADDRAWPIXELS
