
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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

        bool initFont( const std::string& fontName = BitmapFont::normal );
        void draw( const std::string& text ) const;

    private:

        Window* const _window;
        GLuint        _lists;

        bool _initFontGLX( const std::string& fontName );
        bool _initFontWGL( const std::string& fontName );
        bool _initFontAGL( const std::string& fontName );

        void _setupLists( const GLsizei num );
    };
}
}
#endif  // EQUTIL_BITMAPFONT_H
