
/* Copyright (c) 2011, Daniel Pfeifer <daniel@pfeifer-mail.de>
 *               2011-2012, Stefan Eilemann <eile@eyescale.ch>
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
#include <eq/client/defines.h>
#ifdef AGL

#include <AvailabilityMacros.h>
#undef DEPRECATED_ATTRIBUTE
#define DEPRECATED_ATTRIBUTE

#include "../windowSystem.h"

#include "eventHandler.h"
#include "messagePump.h"
#include "pipe.h"
#include "window.h"

#include <eq/client/os.h>
#include <eq/fabric/gpuInfo.h>
#include <lunchbox/debug.h>

#define MAX_GPUS 32

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
namespace eq
{
namespace agl
{

static class : WindowSystemIF
{
    std::string getName() const { return "AGL"; }

    eq::SystemWindow* createWindow( eq::Window* window ) const
    {
        LBINFO << "Using agl::Window" << std::endl;
        return new Window(window);
    }

    eq::SystemPipe* createPipe( eq::Pipe* pipe ) const
    {
        LBINFO << "Using agl::Pipe" << std::endl;
        return new Pipe(pipe);
    }

    eq::MessagePump* createMessagePump() const
    {
        return new MessagePump;
    }

    bool setupFont( ObjectManager& gl, const void* key, const std::string& name,
                    const uint32_t size ) const
    {
        AGLContext context = aglGetCurrentContext();
        LBASSERT( context );
        if( !context )
        {
            LBWARN << "No AGL context current" << std::endl;
            return false;
        }

        CFStringRef cfFontName = name.empty() ?
            CFStringCreateWithCString( kCFAllocatorDefault, "Georgia",
                                       kCFStringEncodingMacRoman ) :
            CFStringCreateWithCString( kCFAllocatorDefault, name.c_str(),
                                       kCFStringEncodingMacRoman );

        ATSFontFamilyRef font = ATSFontFamilyFindFromName( cfFontName, 
                                                       kATSOptionFlagsDefault );
        CFRelease( cfFontName );

        if( font == 0 )
        {
            LBWARN << "Can't load font " << name << ", using Georgia"
                   << std::endl;
            cfFontName = 
                CFStringCreateWithCString( kCFAllocatorDefault, "Georgia",
                                           kCFStringEncodingMacRoman );

            font = ATSFontFamilyFindFromName( cfFontName,
                                              kATSOptionFlagsDefault );
            CFRelease( cfFontName );
        }
        LBASSERT( font );

        const GLuint lists = _setupLists( gl, key, 256 );
        if( aglUseFont( context, font, normal, size, 0, 256, (long)lists ))
            return true;

        _setupLists( gl, key, 0 );
        return false;
    }

    void configInit( eq::Node* node )
    {
#ifdef EQUALIZER_USE_MAGELLAN
        EventHandler::initMagellan(node);
#endif
    }

    void configExit( eq::Node* node )
    {
#ifdef EQUALIZER_USE_MAGELLAN
        EventHandler::exitMagellan(node);
#endif
    }
} _aglFactory;

}
}
#endif // AGL
