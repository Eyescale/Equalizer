
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com>
 *               2013, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQ_PLUGIN_COMPRESSORYUV
#define EQ_PLUGIN_COMPRESSORYUV

#include "compressor.h"

#include <eq/util/texture.h>
#include <eq/gl.h>
#include <eq/util/types.h>
#include <lunchbox/buffer.h>

namespace eq
{
namespace plugin
{

class CompressorYUV : public Compressor
{
public:
    CompressorYUV();
    virtual ~CompressorYUV();

    static void* getNewCompressor( const unsigned )
        { return new CompressorYUV; }
    static void* getNewDecompressor( const unsigned )
        { return new CompressorYUV; }

    void compress( const void* const, const eq_uint64_t, const bool ) override
        { LBDONTCALL; }

    static bool isCompatible( const GLEWContext* );

    void download( const GLEWContext*, const eq_uint64_t*, const unsigned,
                   const eq_uint64_t, eq_uint64_t*, void** ) override;

    void upload( const GLEWContext*, const void*, const eq_uint64_t*,
                 const eq_uint64_t, const eq_uint64_t*,
                 const unsigned ) override;

protected:
    GLuint   _program;
    lunchbox::Bufferb buffer;

private:
    void _initShader( const GLEWContext*, const char* );
    void _compress( const GLEWContext*, const eq_uint64_t*, eq_uint64_t* );
    void _decompress( const GLEWContext*, const eq_uint64_t* );
    void _download( void* );

    util::FrameBufferObject* _fbo;
    util::Texture* _texture;

};

}
}
#endif //EQ_PLUGIN_COMPRESSORYUV
