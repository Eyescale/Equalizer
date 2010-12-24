
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#include <co/base/errorRegistry.h>
#include <co/base/global.h>

namespace eq
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
    { ERROR_FRAMEBUFFER_FULL_COLOR_TEXTURES,
      "Too many color textures, can't add another one" },
    { ERROR_FRAMEBUFFER_INITIALIZED, "FBO already initialized" },

    { ERROR_CUDACONTEXT_DEVICE_NOTFOUND,
      "Device not found, not enough CUDA devices" },
    { ERROR_CUDACONTEXT_INIT_FAILED,
      "CUDA initialization failed (see client log for more information)" },
    { ERROR_CUDACONTEXT_MISSING_SUPPORT,
      "Client library compiled without CUDA support" },

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
    { ERROR_GLXWINDOW_CREATEPBUFFER_FAILED, "Can't create glX PBuffer" },

    { ERROR_WGLWINDOW_NO_DRAWABLE, "Missing WGL drawable" },
    { ERROR_WGLWINDOW_SETPIXELFORMAT_FAILED, "Can't set window pixel format" },
    { ERROR_WGLWINDOW_REGISTERCLASS_FAILED, "Can't register window class" },
    { ERROR_WGLWINDOW_FULLSCREEN_FAILED, "Can't switch to fullscreen mode" },
    { ERROR_WGLWINDOW_CREATEWINDOW_FAILED, "Can't create window" },
    { ERROR_WGLWINDOW_ARB_PBUFFER_REQUIRED, "WGL_ARB_pbuffer not supported" },
    { ERROR_WGLWINDOW_ARB_FLOAT_FB_REQUIRED,
      "Floating-point framebuffer not supported" },
    { ERROR_WGLWINDOW_CREATEPBUFFER_FAILED, "Can't create PBuffer" },
    { ERROR_WGLWINDOW_SETAFFINITY_PF_FAILED, "Can't set affinity pixel format"},
    { ERROR_WGLWINDOW_CHOOSE_PF_ARB_FAILED,
      "Can't choose pixel format using ARB extension"},
    { ERROR_WGLWINDOW_CREATECONTEXT_FAILED, "Can't create WGL context" },

    { ERROR_CHANNEL_WINDOW_NOTRUNNING, "Window not running" },

    { 0, "" } // last!
};
}

void _initErrors()
{
    co::base::ErrorRegistry& registry = co::base::Global::getErrorRegistry();

    for( size_t i=0; _errors[i].code != 0; ++i )
        registry.setString( _errors[i].code, _errors[i].text );
}

void _exitErrors()
{
    co::base::ErrorRegistry& registry = co::base::Global::getErrorRegistry();

    for( size_t i=0; _errors[i].code != 0; ++i )
        registry.eraseString( _errors[i].code );
}

}

