
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

// HACK: Get rid of deprecated warning for aglUseFont
//   -Wno-deprecated-declarations would do as well, but here it is more isolated
#ifdef AGL
#  include <AvailabilityMacros.h>
#  undef DEPRECATED_ATTRIBUTE
#  define DEPRECATED_ATTRIBUTE
#endif

#include "bitmapFont.h"

#include <eq/client/pipe.h>
#include <eq/client/window.h>

#ifdef WGL
#  include <eq/client/wglWindow.h>
#endif
#ifdef AGL
#  include <eq/client/aglWindow.h>
#endif
#ifdef GLX
#  include <eq/client/glXPipe.h>
#endif

using namespace std;

namespace eq
{
namespace util
{
const string BitmapFont::normal( "EQ_UTIL_BITMAPFONT_NORMAL" );

BitmapFont::BitmapFont( Window* window )
        : _window( window )
        , _lists ( Window::ObjectManager::INVALID )
{
    EQASSERT( window );
}


BitmapFont::~BitmapFont()
{
}

bool BitmapFont::initFont( const std::string& name, const uint32_t size )
{
    const Pipe* pipe = _window->getPipe();
    switch( pipe->getWindowSystem( ))
    {
        case WINDOW_SYSTEM_GLX:
            return _initFontGLX( name, size );
        case WINDOW_SYSTEM_WGL:
            return _initFontWGL( name, size );
        case WINDOW_SYSTEM_AGL:
            return _initFontAGL( name, size );
        default:
            return false;
    }
}

bool BitmapFont::_initFontGLX( const std::string& name,
                               const uint32_t size )
{
#ifdef GLX
    Pipe*    pipe    = _window->getPipe();
    EQASSERT( pipe );
    EQASSERT( pipe->getOSPipe( ));
    EQASSERT( dynamic_cast< const GLXPipe* >( pipe->getOSPipe( )));
    const GLXPipe* osPipe = static_cast< const GLXPipe* >( pipe->getOSPipe( ));

    Display* display = osPipe->getXDisplay();
    EQASSERT( display );

    // see xfontsel
    stringstream font;
    font << "-*-";

    if( name == normal )
        font << "times";
    else
        font << name;
    font << "-*-r-*-*-" << size << "-*-*-*-*-*-*-*";

    XFontStruct* fontStruct = XLoadQueryFont( display, font.str().c_str( )); 
    if( !fontStruct )
    {
        EQWARN << "Can't load font " << font.str() << ", using fixed" << endl;
        fontStruct = XLoadQueryFont( display, "fixed" ); 
    }

    EQASSERT( fontStruct );

    _setupLists( 127 );
    glXUseXFont( fontStruct->fid, 0, 127, _lists );

    XFreeFont( display, fontStruct );
    return true;
#else
    return false;
#endif
}

bool BitmapFont::_initFontWGL( const std::string& name,
                               const uint32_t size )
{
#ifdef WGL
    const OSWindow*    osWindow  = _window->getOSWindow();
    const WGLWindowIF* wglWindow = dynamic_cast< const WGLWindowIF* >(osWindow);

    if( !wglWindow )
    {
        EQWARN << "Window does not use a WGL window" << endl;
        return false;
    }

    HDC dc = wglWindow->getWGLDC();
    if( !dc )
    {
        EQWARN << "WGL window does not have a device context" << endl;
        return false;
    }

    LOGFONT font;
    memset( &font, 0, sizeof( font ));
    font.lfHeight = -size;
    font.lfWeight = FW_NORMAL;
    font.lfCharSet = ANSI_CHARSET;
    font.lfOutPrecision = OUT_DEFAULT_PRECIS;
    font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    font.lfQuality = DEFAULT_QUALITY;
    font.lfPitchAndFamily = FF_DONTCARE | DEFAULT_QUALITY;

    if( name == normal )
        strncpy( font.lfFaceName, "Times New Roman", LF_FACESIZE );
    else
        strncpy( font.lfFaceName, name.c_str(), LF_FACESIZE );

    font.lfFaceName[ LF_FACESIZE-1 ] = '\0';

    HFONT newFont = CreateFontIndirect( &font );
    if( !newFont )
    {
        EQWARN << "Can't load font " << name << ", using Times New Roman" 
               << endl;

        strncpy( font.lfFaceName, "Times New Roman", LF_FACESIZE );
        newFont = CreateFontIndirect( &font );
    }
    EQASSERT( newFont );

    HFONT oldFont = static_cast< HFONT >( SelectObject( dc, newFont ));

    _setupLists( 256 );
    const bool ret = wglUseFontBitmaps( dc, 0 , 255, _lists );
    
    SelectObject( dc, oldFont );
    //DeleteObject( newFont );
    
    if( !ret )
        _setupLists( 0 );

    return ret;
#else
    return false;
#endif
}

bool BitmapFont::_initFontAGL( const std::string& name,
                               const uint32_t size )
{
#ifdef AGL
    const OSWindow*    osWindow  = _window->getOSWindow();
    const AGLWindowIF* aglWindow = dynamic_cast< const AGLWindowIF* >(osWindow);

    if( !aglWindow )
    {
        EQWARN << "Window does not use an AGL window" << endl;
        return false;
    }

    AGLContext context = aglWindow->getAGLContext();
    EQASSERT( context );

    CFStringRef cfFontName;
    if( name == normal )
        cfFontName = CFStringCreateWithCString( kCFAllocatorDefault, "Georgia",
                                                kCFStringEncodingMacRoman );
    else
        cfFontName = 
            CFStringCreateWithCString( kCFAllocatorDefault, name.c_str(),
                                   kCFStringEncodingMacRoman );

    ATSFontFamilyRef font = ATSFontFamilyFindFromName( cfFontName, 
                                                       kATSOptionFlagsDefault );
    CFRelease( cfFontName );

    if( font == 0 )
    {
        EQWARN << "Can't load font " << name << ", using Georgia" << endl;
        cfFontName = 
            CFStringCreateWithCString( kCFAllocatorDefault, "Georgia",
                                       kCFStringEncodingMacRoman );

        font = ATSFontFamilyFindFromName( cfFontName, kATSOptionFlagsDefault );
        CFRelease( cfFontName );
    }
    EQASSERT( font );

    _setupLists( 127 );
    if( !aglUseFont( context, font, ::normal, size, 0, 256, (long)_lists ))
    {
        _setupLists( 0 );
        return false;
    }
    
    return true;
#else
    return false;
#endif
}

void BitmapFont::_setupLists( const GLsizei num )
{
    Window::ObjectManager* gl = _window->getObjectManager();
    EQASSERT( gl );

    if( _lists != Window::ObjectManager::INVALID )
        gl->deleteList( this );

    if( num == 0 )
        _lists = Window::ObjectManager::INVALID;
    else
    {
        _lists = gl->newList( this, num );
        EQASSERT( _lists != Window::ObjectManager::INVALID );
    }
}

void BitmapFont::draw( const std::string& text ) const
{
    const Pipe* pipe = _window->getPipe();
    switch( pipe->getWindowSystem( ))
    {
        case WINDOW_SYSTEM_GLX:
        case WINDOW_SYSTEM_AGL:
        case WINDOW_SYSTEM_WGL:
            if( _lists != Window::ObjectManager::INVALID )
            {
                glListBase( _lists );
                glCallLists( text.size(), GL_UNSIGNED_BYTE, text.c_str( ));
                glListBase( 0 );
            }
            break;

        default:
            EQUNIMPLEMENTED;
    }
}

}
}
