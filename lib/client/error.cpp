
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

#include <eq/base/errorRegistry.h>
#include <eq/base/global.h>

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
    { ERROR_AGL_MISSING_SUPPORT, "Client library compiled without AGL support"},
    { ERROR_GLX_MISSING_SUPPORT, "Client library compiled without GLX support"},

    { ERROR_PIPE_NODE_NOTRUNNING, "Node not running" },

    { ERROR_AGLPIPE_DISPLAYS_NOTFOUND, "Can't get display identifier list" },
    { ERROR_AGLPIPE_DEVICE_NOTFOUND, "Can't get display identifier for device"},

    { ERROR_GLXPIPE_DEVICE_NOTFOUND, "Can't open display" },
    { ERROR_GLXPIPE_GLX_NOTFOUND, "Display does not support GLX" },

    { ERROR_WGLPIPE_AFFINITYDC_FAILED, "Can't create affinity DC" },
    { ERROR_WGLPIPE_ENUMDISPLAYS_FAILED, "Can't enumerate display devices" },
    { ERROR_WGLPIPE_CREATEDC_FAILED, "Can't create device context" },
    { ERROR_WGLPIPE_ENUMGPUS_FAILED, "Can't enumerate GPU" },
    { ERROR_WGLPIPE_REGISTERCLASS_FAILED,
      "Can't register temporary window class" },
    { ERROR_WGLPIPE_CREATEWINDOW_FAILED, "Can't create temporary window" },
    { ERROR_WGLPIPE_CHOOSEPF_FAILED, "Can't find temporary pixel format" },
    { ERROR_WGLPIPE_SETPF_FAILED, "Can't set temporary pixel format" },
    { ERROR_WGLPIPE_CREATECONTEXT_FAILED,
      "Can't create temporary OpenGL context" },
    { ERROR_WGLPIPE_WGLEWINIT_FAILED, "Pipe WGLEW initialization failed" },
    { 0, "" } // last!
};
}

void _initErrors()
{
    base::ErrorRegistry& registry = base::Global::getErrorRegistry();

    for( size_t i=0; _errors[i].code != 0; ++i )
        registry.setString( _errors[i].code, _errors[i].text );
}

void _exitErrors()
{
    base::ErrorRegistry& registry = base::Global::getErrorRegistry();

    for( size_t i=0; _errors[i].code != 0; ++i )
        registry.eraseString( _errors[i].code );
}

}

