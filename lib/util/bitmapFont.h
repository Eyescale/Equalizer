
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
#include <eq/client/windowSystem.h> // GL prototypes

#include <string>

namespace eq
{
class Window;

/** 
 * @namespace eq::util
 * @brief Equalizer utility classes
 *
 * The eq::util namespace groups common utility classes.
 */
namespace util
{
    /** A wrapper around agl, wgl and glx bitmap fonts. */
    class EQ_EXPORT BitmapFont
    {
    public:
        BitmapFont( Window* window );
        ~BitmapFont();

        const static std::string normal; //!< a normal default font

        bool initFont( const std::string& name = normal,
                       const uint32_t size = 12 );
        void draw( const std::string& text ) const;

    private:

        Window* const _window;
        GLuint        _lists;

        bool _initFontGLX( const std::string& name, const uint32_t size );
        bool _initFontWGL( const std::string& name, const uint32_t size );
        bool _initFontAGL( const std::string& name, const uint32_t size );

        void _setupLists( const GLsizei num );
    };
}
}
#endif  // EQUTIL_BITMAPFONT_H
