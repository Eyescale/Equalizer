
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

// HACK: Get rid of deprecated warning for aglUseFont
//   -Wno-deprecated-declarations would do as well, but here it is more isolated
#ifdef AGL
#  include <AvailabilityMacros.h>
#  undef DEPRECATED_ATTRIBUTE
#  define DEPRECATED_ATTRIBUTE
#endif

#include "bitmapFont.h"

#include <eq/client/window.h>

#ifdef AGL
#  include <eq/client/aglWindow.h>
#endif

using namespace std;

namespace eq
{
namespace util
{
const string BitmapFont::normal( "EQ_UTIL_BITMAPFONT_NORMAL" );

BitmapFont::BitmapFont( Window* window )
        : _window( window )
        , _lists ( Window::ObjectManager::FAILED )
{
    EQASSERT( window );
}


BitmapFont::~BitmapFont()
{
}

bool BitmapFont::setFont( const string& fontName )
{
    const Pipe* pipe = _window->getPipe();
    switch( pipe->getWindowSystem( ))
    {
        case WINDOW_SYSTEM_GLX:
            return _setFontGLX( fontName );
        case WINDOW_SYSTEM_WGL:
            return _setFontWGL( fontName );
        case WINDOW_SYSTEM_AGL:
            return _setFontAGL( fontName );
        default:
            return false;
    }
}

bool BitmapFont::_setFontGLX( const std::string& fontName )
{
#ifdef GLX
    Window::ObjectManager* gl = _window->getObjectManager();
    EQASSERT( gl );

    if( _lists != Window::ObjectManager::FAILED )
        gl->deleteList( _lists );

    _lists = gl->newList( this, 127 );
    EQASSERT( _lists != Window::ObjectManager::FAILED );

    Pipe*    pipe    = _window->getPipe();
    Display* display = pipe->getXDisplay();
    EQASSERT( display );

    string font = fontName;
    if( font == normal )
        font = "-*-times-*-r-*-*-12-*-*-*-*-*-*-*"; // see xfontsel

    XFontStruct* fontStruct = XLoadQueryFont( display, font.c_str( )); 
    if( !fontStruct )
    {
        EQWARN << "Can't load font " << font << ", using fixed" << endl;
        fontStruct = XLoadQueryFont( display, "fixed" ); 
    }

    EQASSERT( fontStruct );
    glXUseXFont( fontStruct->fid, 0, 127, _lists );

    XFreeFont( display, fontStruct );
    return true;
#else
    return false;
#endif
}

bool BitmapFont::_setFontWGL( const std::string& fontName )
{
#ifdef WGL
    EQUNIMPLEMENTED;
    return false;
#else
    return false;
#endif
}

bool BitmapFont::_setFontAGL( const std::string& fontName )
{
#ifdef AGL
    const OSWindow*    osWindow  = _window->getOSWindow();
    const AGLWindowIF* aglWindow = dynamic_cast< const AGLWindowIF* >(osWindow);

    if( !aglWindow )
    {
        EQWARN << "Window does not use an AGL window" << endl;
        return false;
    }

    Window::ObjectManager* gl = _window->getObjectManager();
    EQASSERT( gl );

    if( _lists != Window::ObjectManager::FAILED )
        gl->deleteList( _lists );

    _lists = gl->newList( this, 256 );
    EQASSERT( _lists != Window::ObjectManager::FAILED );


    AGLContext context = aglWindow->getAGLContext();
    EQASSERT( context );

    CFStringRef cfFontName;
    if( fontName == normal )
        cfFontName = CFStringCreateWithCString( kCFAllocatorDefault, "Georgia",
                                                kCFStringEncodingMacRoman );
    else
        cfFontName = 
            CFStringCreateWithCString( kCFAllocatorDefault, fontName.c_str(),
                                   kCFStringEncodingMacRoman );

    ATSFontFamilyRef font = ATSFontFamilyFindFromName( cfFontName, 
                                                       kATSOptionFlagsDefault );
    CFRelease( cfFontName );

    if( font == 0 )
    {
        EQWARN << "Can't load font " << fontName << ", using Georgia" << endl;
        cfFontName = 
            CFStringCreateWithCString( kCFAllocatorDefault, "Georgia",
                                       kCFStringEncodingMacRoman );

        font = ATSFontFamilyFindFromName( cfFontName, kATSOptionFlagsDefault );
        CFRelease( cfFontName );
    }
    EQASSERT( font );

    if( !aglUseFont( context, font, ::normal, 12, 0, 256, (long)_lists ))
    {
        gl->deleteList( _lists );
        _lists = Window::ObjectManager::FAILED;
        return false;
    }
    
    return true;
#else
    return false;
#endif
}

void BitmapFont::draw( const std::string& text )
{
    const Pipe* pipe = _window->getPipe();
    switch( pipe->getWindowSystem( ))
    {
        case WINDOW_SYSTEM_GLX:
        case WINDOW_SYSTEM_AGL:
            if( _lists != Window::ObjectManager::FAILED )
            {
                glListBase( _lists );
                glCallLists( text.size(), GL_UNSIGNED_BYTE, text.c_str( ));
                glListBase( 0 );
            }
            break;

        case WINDOW_SYSTEM_WGL:
            break;

        default:
            EQUNIMPLEMENTED;
    }
}

}
}
