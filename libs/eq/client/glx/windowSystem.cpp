
/* Copyright (c) 2011 Daniel Pfeifer <daniel@pfeifer-mail.de>
 *               2011, Stefan Eilemann <eile@eyescale.ch>
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

#include "../windowSystem.h"

#include "window.h"
#include "pipe.h"
#include "messagePump.h"

#include <eq/fabric/gpuInfo.h>

namespace eq
{
namespace glx
{

namespace
{
static bool _queryDisplay( const std::string display, GPUInfo& info )
{
    EQASSERT( !display.empty( ));

    ::Display* xDisplay = XOpenDisplay( display.c_str( ));
    if( !xDisplay )
        return false;

    int major, event, error;
    if( !XQueryExtension( xDisplay, "GLX", &major, &event, &error ))
    {
        XCloseDisplay( xDisplay );
        return false;
    }

    EQCHECK( Pipe::getGPUInfo( xDisplay, info ));
    XCloseDisplay( xDisplay );
    return true;
}

}

static class : WindowSystemIF
{
    std::string getName() const { return "GLX"; }

    eq::SystemWindow* createWindow(eq::Window* window) const
    {
        EQINFO << "Using glx::Window" << std::endl;
        return new Window(window);
    }

    eq::SystemPipe* createPipe(eq::Pipe* pipe) const
    {
        EQINFO << "Using glx::Pipe" << std::endl;
        return new Pipe(pipe);
    }

    eq::MessagePump* createMessagePump() const
    {
        return new MessagePump;
    }

    GPUInfos discoverGPUs() const
    {
        GPUInfos result;
        GPUInfo defaultInfo;

        const char* displayEnv = getenv( "DISPLAY" );
        if( displayEnv && displayEnv[0] != '\0' )
        {
            const std::string display( displayEnv );
            if( _queryDisplay( display, defaultInfo ))
            {
                if( display[0] != ':' )
                {
                    defaultInfo.port = EQ_UNDEFINED_UINT32;
                    defaultInfo.device = EQ_UNDEFINED_UINT32;
                }
                result.push_back( defaultInfo );
            }
        }

        for( uint32_t i = 0; i < EQ_MAX_UINT32; ++i ) // x servers
            for( uint32_t j = 0; j < EQ_MAX_UINT32; ++j ) // x screens
            {
                std::stringstream stream;
                stream <<  ':' << i << '.' << j;
                
                GPUInfo info;
                if( _queryDisplay( stream.str(), info ))
                {
                    EQASSERTINFO( i == info.port, i << ", " << info );
                    EQASSERT( j == info.device );

                    if( info != defaultInfo )
                        result.push_back( info );
                }
                else if( j == 0 ) // X Server does not exist, stop query
                    return result;
                else // X Screen does not exist, try next server
                    break;
            }

        EQASSERTINFO( 0, "Unreachable" );
        return result;
    }

} _glXFactory;

}
}
