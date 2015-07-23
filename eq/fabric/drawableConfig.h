
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_DRAWABLECONFIG_H
#define EQ_DRAWABLECONFIG_H

#include <iostream>

namespace eq
{
namespace fabric
{
    /** Stores the characteristics of a window's frame buffer configuration. */
    struct DrawableConfig
    {
        DrawableConfig()
                : stencilBits(0), colorBits(0), alphaBits(0), accumBits(0)
                , glVersion( 0.f ), glewGLVersion( 0.f ), stereo( false )
                , doublebuffered( false ) , coreProfile( false ) {}

        int32_t stencilBits;    //!< Number of stencil bits
        int32_t colorBits;      //!< Number of bits per color component
        int32_t alphaBits;      //!< Number of alpha bits
        int32_t accumBits;      //!< Number of accumulation bits
        float   glVersion;      //!< OpenGL version (glGetString( GL_VERSION ))
        float   glewGLVersion;  //!< OpenGL version (GLEW detected)
        bool    stereo;         //!< Active stereo supported
        bool    doublebuffered; //!< Doublebuffering supported
        bool    coreProfile;    //!< Core or Compat profile (since OpenGL 3.2)
    };

    inline std::ostream& operator << ( std::ostream& os,
                                       const DrawableConfig& config )
    {
        os << "GL" << config.glVersion << "|GLEW" << config.glewGLVersion;
        if( config.glVersion >= 3.2f )
            os << (config.coreProfile ? "|Core" : "|Compat");
        os << "|rgb" << config.colorBits;
        if( config.alphaBits )
            os << "a" << config.alphaBits;
        if( config.stencilBits )
            os << "|st" << config.stencilBits;
        if( config.accumBits )
            os << "|acc" << config.accumBits;
        if( config.doublebuffered )
            os << "|DB";
        if( config.stereo )
            os << "|ST";
        return os;
    }
}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::DrawableConfig& value )
{
    byteswap( value.stencilBits );
    byteswap( value.colorBits );
    byteswap( value.alphaBits );
    byteswap( value.accumBits );
    byteswap( value.glVersion );
}
}
#endif // EQ_DRAWABLECONFIG_H
