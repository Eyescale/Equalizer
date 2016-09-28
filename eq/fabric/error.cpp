
/* Copyright (c) 2010-2016, Stefan Eilemann <eile@eyescale.ch>
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

#include "error.h"

#include "errorRegistry.h"
#include "global.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace fabric
{

namespace
{
struct ErrorData
{
    const uint32_t code;
    const std::string text;
};

ErrorData _errors[] = {
    { ERROR_FBO_UNSUPPORTED, "Framebuffer objects not supported" },
    { ERROR_FRAMEBUFFER_STATUS, "Error querying framebuffer status" },
    { ERROR_FRAMEBUFFER_UNSUPPORTED, "Unsupported framebuffer format" },
    { ERROR_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
      "Framebuffer incomplete, missing attachment" },
    { ERROR_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
      "Framebuffer incomplete, incomplete attachment" },
    { ERROR_FRAMEBUFFER_INCOMPLETE_DIMENSIONS,
      "Framebuffer incomplete, attached images must have same dimensions" },
    { ERROR_FRAMEBUFFER_INCOMPLETE_FORMATS,
      "Framebuffer incomplete, attached images must have same format" },
    { ERROR_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
      "Framebuffer incomplete, missing draw buffer" },
    { ERROR_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
      "Framebuffer incomplete, missing read buffer" },
    { ERROR_FRAMEBUFFER_INITIALIZED, "FBO already initialized" },
    { ERROR_FRAMEBUFFER_INVALID_SIZE, "FBO size not supported" },
    { ERROR_FRAMEBUFFER_INVALID_SAMPLES, "Multisampled FBO not supported" },

    { ERROR_WINDOWSYSTEM_UNKNOWN, "Unknown windowing system" },

    { ERROR_NODE_LAUNCH, "Execution of node launch command failed" },
    { ERROR_NODE_CONNECT, "Node process did not start" },

    { ERROR_PIPE_NODE_NOTRUNNING, "Node not running" },

    { ERROR_SYSTEMPIPE_PIXELFORMAT_NOTFOUND,
      "Can't find temporary pixel format" },
    { ERROR_SYSTEMPIPE_CREATECONTEXT_FAILED,
      "Can't create temporary OpenGL context" },
    { ERROR_SYSTEMPIPE_CREATEWINDOW_FAILED, "Can't create temporary window" },

    { ERROR_AGLPIPE_DISPLAYS_NOTFOUND, "Can't get display identifier list" },
    { ERROR_AGLPIPE_DEVICE_NOTFOUND, "Can't get display identifier for device"},

    { ERROR_GLXPIPE_DEVICE_NOTFOUND, "Can't open display" },
    { ERROR_GLXPIPE_GLX_NOTFOUND, "Display does not support GLX" },
    { ERROR_GLXPIPE_GLXEWINIT_FAILED, "Pipe GLXEW initialization failed" },

    { ERROR_WGL_CREATEAFFINITYDC_FAILED, "Can't create affinity DC" },
    { ERROR_WGLPIPE_ENUMDISPLAYS_FAILED, "Can't enumerate display devices" },
    { ERROR_WGLPIPE_CREATEDC_FAILED, "Can't create device context" },
    { ERROR_WGLPIPE_ENUMGPUS_FAILED, "Can't enumerate GPU" },
    { ERROR_WGLPIPE_REGISTERCLASS_FAILED,
      "Can't register temporary window class" },
    { ERROR_WGLPIPE_SETPF_FAILED, "Can't set temporary pixel format" },
    { ERROR_WGLPIPE_WGLEWINIT_FAILED, "Pipe WGLEW initialization failed" },

    { ERROR_WINDOW_PIPE_NOTRUNNING, "Pipe not running" },
    { ERROR_WINDOW_PVP_INVALID, "Invalid window pixel viewport" },

    { ERROR_SYSTEMWINDOW_PIXELFORMAT_NOTFOUND,
      "Can't find matching pixel format" },
    { ERROR_SYSTEMWINDOW_NO_PIXELFORMAT, "Missing pixel format" },
    { ERROR_SYSTEMWINDOW_ARB_FLOAT_FB_REQUIRED,
      "Floating-point framebuffer not supported" },

    { ERROR_AGLWINDOW_NO_CONTEXT, "Missing AGL context" },
    { ERROR_AGLWINDOW_CREATECONTEXT_FAILED, "Can't create AGL context" },
    { ERROR_AGLWINDOW_CREATEPBUFFER_FAILED, "Can't create AGL PBuffer" },
    { ERROR_AGLWINDOW_SETPBUFFER_FAILED, "Can't set AGL PBuffer" },
    { ERROR_AGLWINDOW_CREATEWINDOW_FAILED, "Can't create Carbon window" },
    { ERROR_AGLWINDOW_SETWINDOW_FAILED, "Can't set Carbon window" },

    { ERROR_GLXWINDOW_NO_DRAWABLE, "Missing GLX drawable" },
    { ERROR_GLXWINDOW_NO_DISPLAY, "Missing X11 display connection" },
    { ERROR_GLXWINDOW_CREATECONTEXT_FAILED, "Can't create glX context" },
    { ERROR_GLXWINDOW_CREATEWINDOW_FAILED, "Can't create X11 window" },
    { ERROR_GLXWINDOW_GLXQUERYVERSION_FAILED, "Can't get GLX version" },
    { ERROR_GLXWINDOW_GLX_1_3_REQUIRED, "Need at least GLX 1.3" },
    { ERROR_GLXWINDOW_NO_FBCONFIG, "Can't find FBConfig for visual" },
    { ERROR_GLXWINDOW_NO_VISUAL, "FBConfig has no associated visual" },
    { ERROR_GLXWINDOW_CREATEPBUFFER_FAILED, "Can't create glX PBuffer" },
    { ERROR_GLXWINDOW_FBCONFIG_REQUIRED,
      "Can't find FBConfig functions (GLX 1.3 or GLX_SGIX_fbconfig" },

    { ERROR_WGLWINDOW_NO_DRAWABLE, "Missing WGL drawable" },
    { ERROR_WGLWINDOW_SETPIXELFORMAT_FAILED, "Can't set window pixel format" },
    { ERROR_WGLWINDOW_REGISTERCLASS_FAILED, "Can't register window class" },
    { ERROR_WGLWINDOW_FULLSCREEN_FAILED, "Can't switch to fullscreen mode" },
    { ERROR_WGLWINDOW_CREATEWINDOW_FAILED, "Can't create window" },
    { ERROR_WGLWINDOW_ARB_PBUFFER_REQUIRED, "WGL_ARB_pbuffer not supported" },
    { ERROR_WGLWINDOW_CREATEPBUFFER_FAILED, "Can't create PBuffer" },
    { ERROR_WGLWINDOW_SETAFFINITY_PF_FAILED, "Can't set affinity pixel format"},
    { ERROR_WGLWINDOW_CHOOSE_PF_ARB_FAILED,
      "Can't choose pixel format using ARB extension"},
    { ERROR_WGLWINDOW_CREATECONTEXT_FAILED, "Can't create WGL context" },

    { ERROR_CHANNEL_WINDOW_NOTRUNNING, "Window not running" },

    { ERROR_PBO_UNSUPPORTED, "Pixel Buffer Objects not supported" },
    { ERROR_PBO_NOT_INITIALIZED, "PBO is not initialized" },
    { ERROR_PBO_SIZE_TOO_SMALL, "PBO size is too small, it has to be > 0" },
    { ERROR_PBO_TYPE_UNSUPPORTED, "Unsupported PBO type" },

    { 0, "" } // last!
};
}

void _initErrors()
{
    eq::fabric::ErrorRegistry& registry =eq::fabric::Global::getErrorRegistry();
    for( size_t i=0; _errors[i].code != 0; ++i )
        registry.setString( _errors[i].code, _errors[i].text );
}

void _exitErrors()
{
    eq::fabric::ErrorRegistry& registry =eq::fabric::Global::getErrorRegistry();
    for( size_t i=0; _errors[i].code != 0; ++i )
        registry.eraseString( _errors[i].code );
}

Error::Error()
    : _code( ERROR_NONE )
    , _originator()
{}

Error::Error( const uint32_t code, const uint128_t& originator )
    : _code( code )
    , _originator( originator )
{}

Error& Error::operator = ( const ErrorCode code )
{
    _code = code;
    return *this;
}

Error::operator bool_t() const
{
    return _code != ERROR_NONE ? &Error::bool_true : 0;
}

bool Error::operator ! () const
{
    return _code == ERROR_NONE;\
}

uint32_t Error::getCode() const
{
    return _code;
}

const uint128_t& Error::getOriginator() const
{
    return _originator;
}

bool Error::operator == ( const Error& rhs ) const
{
    return _code == rhs._code;
}

bool Error::operator != ( const Error& rhs ) const
{
    return _code != rhs._code;
}

bool Error::operator == ( const uint32_t code ) const
{
    return _code == code;
}

bool Error::operator != ( const uint32_t code ) const
{
    return _code != code;
}

void Error::serialize( co::DataOStream& os ) const
{
    os << _code << _originator;
}

void Error::deserialize( co::DataIStream& is )
{
    is >> _code >> _originator;
}

std::ostream& operator << ( std::ostream& os, const Error& error )
{
    const ErrorRegistry& registry = Global::getErrorRegistry();
    const std::string& text = registry.getString( error.getCode( ));

    if( text.empty( ))
        return os << "error 0x" << std::hex << uint32_t( error.getCode( ))
                  << std::dec;

    return os << text << " (0x" << std::hex << uint32_t( error.getCode( ))
              << std::dec << ")";
}

} // fabric
} // eq
