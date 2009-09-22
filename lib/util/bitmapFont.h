
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQUTIL_BITMAPFONT_H
#define EQUTIL_BITMAPFONT_H

#include <eq/base/base.h>
#include <eq/client/os.h> // GL prototypes

#include <string>

namespace eq
{
class Window;
template< typename T > class ObjectManager;

/** 
 * @namespace eq::util
 * @brief Equalizer utility classes
 *
 * The eq::util namespace groups common utility classes.
 */
namespace util
{
    /** A wrapper around agl, wgl and glx bitmap fonts. */
    template< typename OMT > class BitmapFont
    {
    public:
        EQ_EXPORT BitmapFont( ObjectManager< OMT >& gl, const OMT& key );
        EQ_EXPORT ~BitmapFont();

        EQ_EXPORT bool init( Window* window, const std::string& name,
                             const uint32_t size = 12 );
        EQ_EXPORT void exit();
        EQ_EXPORT void draw( const std::string& text ) const;

    private:
        ObjectManager< OMT >& _gl;
        const OMT             _key;

        bool _initGLX( Window* window, const std::string& name,
                       const uint32_t size );
        bool _initWGL( Window* window, const std::string& name,
                       const uint32_t size );
        bool _initAGL( Window* window, const std::string& name,
                       const uint32_t size );

        GLuint _setupLists( const GLsizei num );
    };
}
}
#endif  // EQUTIL_BITMAPFONT_H
