
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

namespace util
{
    /** A wrapper around agl, wgl and glx bitmap fonts. */
    class EQ_EXPORT BitmapFont
    {
    public:
        BitmapFont( Window* window );
        ~BitmapFont();

        const static std::string normal; //!< a normal default font

        bool setFont( const std::string& fontName = BitmapFont::normal );
        void draw( const std::string& text );

    private:

        Window* const _window;
        GLuint        _lists;

        bool _setFontGLX( const std::string& fontName );
        bool _setFontWGL( const std::string& fontName );
        bool _setFontAGL( const std::string& fontName );
    };
}
}
#endif  // EQUTIL_BITMAPFONT_H
