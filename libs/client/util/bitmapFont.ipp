
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <eq/defines.h>
#ifdef AGL
#  include <AvailabilityMacros.h>
#  undef DEPRECATED_ATTRIBUTE
#  define DEPRECATED_ATTRIBUTE
#endif

#include "bitmapFont.h"

#include "objectManager.h"

#include <co/base/debug.h>
#include <co/base/lock.h>
#include <co/base/log.h>
#include <co/base/scopedMutex.h>

namespace eq
{
namespace util
{

template< class OMT >
BitmapFont< OMT >::BitmapFont( ObjectManager< OMT >& gl, const OMT& key )
        // We create a new shared object manager. Typically we are exited by
        // the last user, at which point the given OM may have been deleted
        : _gl( gl.glewGetContext(), &gl )
        , _key( key )
{
}

template< class OMT >
BitmapFont< OMT >::~BitmapFont()
{
    const GLuint lists = _gl.getList( _key );
    if( lists != ObjectManager< OMT >::INVALID )
        EQWARN << "OpenGL BitmapFont was not freed" << std::endl;
}

template< class OMT >
bool BitmapFont< OMT >::init( const WindowSystem ws, const std::string& name,
                              const uint32_t size )
{
    switch( ws )
    {
        case WINDOW_SYSTEM_GLX:
            return _initGLX( name, size );
        case WINDOW_SYSTEM_WGL:
            return _initWGL( name, size );
        case WINDOW_SYSTEM_AGL:
            return _initAGL( name, size );
        default:
            return false;
    }

    EQASSERTINFO( _gl.getList( _key ) != ObjectManager< OMT >::INVALID, 
                  "Font initialization failed" );
}

template< class OMT >
void BitmapFont< OMT >::exit()
{
    _setupLists( 0 );
}

#ifdef GLX
template< class OMT >
bool BitmapFont< OMT >::_initGLX( const std::string& name, const uint32_t size )
{
    Display* display = XGetCurrentDisplay();
    EQASSERT( display );
    if( !display )
    {
        EQWARN << "No current X11 display, use eq::XSetCurrentDisplay()"
               << std::endl;
        return false;
    }

    // see xfontsel
    std::stringstream font;
    font << "-*-";

    if( name.empty( ))
        font << "times";
    else
        font << name;
    font << "-*-r-*-*-" << size << "-*-*-*-*-*-*-*";

    // X11 font initialization is not thread safe. Using a mutex here is not
    // performance-critical
    static co::base::Lock lock;
    co::base::ScopedMutex<> mutex( lock );

    XFontStruct* fontStruct = XLoadQueryFont( display, font.str().c_str( )); 
    if( !fontStruct )
    {
        EQWARN << "Can't load font " << font.str() << ", using fixed"
               << std::endl;
        fontStruct = XLoadQueryFont( display, "fixed" ); 
    }

    EQASSERT( fontStruct );

    const GLuint lists = _setupLists( 127 );
    glXUseXFont( fontStruct->fid, 0, 127, lists );

    XFreeFont( display, fontStruct );
    return true;
}
#else
template< class OMT >
bool BitmapFont< OMT >::_initGLX( const std::string&, const uint32_t )
{
    return false;
}
#endif

#ifdef WGL
template< class OMT >
bool BitmapFont< OMT >::_initWGL( const std::string& name, const uint32_t size )
{
    HDC dc = wglGetCurrentDC();
    if( !dc )
    {
        EQWARN << "No WGL device context current" << std::endl;
        return false;
    }

    LOGFONT font;
    memset( &font, 0, sizeof( font ));
    font.lfHeight = -static_cast< LONG >( size );
    font.lfWeight = FW_NORMAL;
    font.lfCharSet = ANSI_CHARSET;
    font.lfOutPrecision = OUT_DEFAULT_PRECIS;
    font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    font.lfQuality = DEFAULT_QUALITY;
    font.lfPitchAndFamily = FF_DONTCARE | DEFAULT_QUALITY;

    if( name.empty( ))
        strncpy( font.lfFaceName, "Times New Roman", LF_FACESIZE );
    else
        strncpy( font.lfFaceName, name.c_str(), LF_FACESIZE );

    font.lfFaceName[ LF_FACESIZE-1 ] = '\0';

    HFONT newFont = CreateFontIndirect( &font );
    if( !newFont )
    {
        EQWARN << "Can't load font " << name << ", using Times New Roman" 
               << std::endl;

        strncpy( font.lfFaceName, "Times New Roman", LF_FACESIZE );
        newFont = CreateFontIndirect( &font );
    }
    EQASSERT( newFont );

    HFONT oldFont = static_cast< HFONT >( SelectObject( dc, newFont ));

    const GLuint lists = _setupLists( 256 );
    const bool ret = wglUseFontBitmaps( dc, 0 , 255, lists );
    
    SelectObject( dc, oldFont );
    //DeleteObject( newFont );
    
    if( !ret )
        _setupLists( 0 );

    return ret;
}
#else
template< class OMT >
bool BitmapFont< OMT >::_initWGL( const std::string&, const uint32_t )
{
    return false;
}
#endif

#ifdef AGL
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
template< class OMT >
bool BitmapFont< OMT >::_initAGL( const std::string& name, const uint32_t size )
{
    AGLContext context = aglGetCurrentContext();
    EQASSERT( context );
    if( !context )
    {
        EQWARN << "No AGL context current" << std::endl;
        return false;
    }

    CFStringRef cfFontName;
    if( name.empty( ))
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
        EQWARN << "Can't load font " << name << ", using Georgia" << std::endl;
        cfFontName = 
            CFStringCreateWithCString( kCFAllocatorDefault, "Georgia",
                                       kCFStringEncodingMacRoman );

        font = ATSFontFamilyFindFromName( cfFontName, kATSOptionFlagsDefault );
        CFRelease( cfFontName );
    }
    EQASSERT( font );

    const GLuint lists = _setupLists( 127 );
    if( !aglUseFont( context, font, normal, size, 0, 256, (long)lists ))
    {
        _setupLists( 0 );
        return false;
    }
    
    return true;
}
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#else
template< class OMT >
bool BitmapFont< OMT >::_initAGL( const std::string&, const uint32_t )
{
    return false;
}
#endif

template< class OMT >
GLuint BitmapFont< OMT >::_setupLists( const GLsizei num )
{
    GLuint lists = _gl.getList( _key );
    if( lists != ObjectManager< OMT >::INVALID )
        _gl.deleteList( _key );

    if( num == 0 )
        lists = ObjectManager< OMT >::INVALID;
    else
    {
        lists = _gl.newList( _key, num );
        EQASSERT( lists != ObjectManager< OMT >::INVALID );
    }
    return lists;
}

template< class OMT >
void BitmapFont< OMT >::draw( const std::string& text ) const
{
    const GLuint lists = _gl.getList( _key );
    EQASSERTINFO( lists != ObjectManager< OMT >::INVALID, 
                  "Font not initialized" );

    if( lists != ObjectManager< OMT >::INVALID )
    {
        glListBase( lists );
        glCallLists( GLsizei( text.size( )), GL_UNSIGNED_BYTE, text.c_str( ));
        glListBase( 0 );
    }
}
}
}
