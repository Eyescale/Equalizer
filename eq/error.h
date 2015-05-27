
/* Copyright (c) 2010-2015, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQ_ERROR_H
#define EQ_ERROR_H

#include <eq/fabric/error.h>

namespace eq
{
    using fabric::Error;
    using fabric::ErrorCode;
    using fabric::ERROR_NONE;
    using fabric::ERROR_FBO_UNSUPPORTED;
    using fabric::ERROR_FRAMEBUFFER_STATUS;
    using fabric::ERROR_FRAMEBUFFER_UNSUPPORTED;
    using fabric::ERROR_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
    using fabric::ERROR_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
    using fabric::ERROR_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;
    using fabric::ERROR_FRAMEBUFFER_INCOMPLETE_FORMATS;
    using fabric::ERROR_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER;
    using fabric::ERROR_FRAMEBUFFER_INCOMPLETE_READ_BUFFER;
    using fabric::ERROR_FRAMEBUFFER_INITIALIZED;
    using fabric::ERROR_FRAMEBUFFER_INVALID_SIZE;
    using fabric::ERROR_FRAMEBUFFER_INVALID_SAMPLES;
    using fabric::ERROR_CUDACONTEXT_DEVICE_NOTFOUND;
    using fabric::ERROR_CUDACONTEXT_INIT_FAILED;
    using fabric::ERROR_CUDACONTEXT_MISSING_SUPPORT;
    using fabric::ERROR_WINDOWSYSTEM_UNKNOWN;
    using fabric::ERROR_NODE_LAUNCH;
    using fabric::ERROR_NODE_CONNECT;
    using fabric::ERROR_PIPE_NODE_NOTRUNNING;
    using fabric::ERROR_SYSTEMPIPE_PIXELFORMAT_NOTFOUND;
    using fabric::ERROR_SYSTEMPIPE_CREATECONTEXT_FAILED;
    using fabric::ERROR_SYSTEMPIPE_CREATEWINDOW_FAILED;
    using fabric::ERROR_AGLPIPE_DISPLAYS_NOTFOUND;
    using fabric::ERROR_AGLPIPE_DEVICE_NOTFOUND;
    using fabric::ERROR_GLXPIPE_DEVICE_NOTFOUND;
    using fabric::ERROR_GLXPIPE_GLX_NOTFOUND;
    using fabric::ERROR_GLXPIPE_GLXEWINIT_FAILED;
    using fabric::ERROR_WGL_CREATEAFFINITYDC_FAILED;
    using fabric::ERROR_WGLPIPE_ENUMDISPLAYS_FAILED;
    using fabric::ERROR_WGLPIPE_CREATEDC_FAILED;
    using fabric::ERROR_WGLPIPE_ENUMGPUS_FAILED;
    using fabric::ERROR_WGLPIPE_REGISTERCLASS_FAILED;
    using fabric::ERROR_WGLPIPE_SETPF_FAILED;
    using fabric::ERROR_WGLPIPE_WGLEWINIT_FAILED;
    using fabric::ERROR_WINDOW_PIPE_NOTRUNNING;
    using fabric::ERROR_WINDOW_PVP_INVALID;
    using fabric::ERROR_SYSTEMWINDOW_PIXELFORMAT_NOTFOUND;
    using fabric::ERROR_SYSTEMWINDOW_NO_PIXELFORMAT;
    using fabric::ERROR_SYSTEMWINDOW_ARB_FLOAT_FB_REQUIRED;
    using fabric::ERROR_AGLWINDOW_NO_CONTEXT;
    using fabric::ERROR_AGLWINDOW_CREATECONTEXT_FAILED;
    using fabric::ERROR_AGLWINDOW_CREATEPBUFFER_FAILED;
    using fabric::ERROR_AGLWINDOW_SETPBUFFER_FAILED;
    using fabric::ERROR_AGLWINDOW_CREATEWINDOW_FAILED;
    using fabric::ERROR_AGLWINDOW_SETWINDOW_FAILED;
    using fabric::ERROR_GLXWINDOW_NO_DRAWABLE;
    using fabric::ERROR_GLXWINDOW_NO_DISPLAY;
    using fabric::ERROR_GLXWINDOW_CREATECONTEXT_FAILED;
    using fabric::ERROR_GLXWINDOW_CREATEWINDOW_FAILED;
    using fabric::ERROR_GLXWINDOW_GLXQUERYVERSION_FAILED;
    using fabric::ERROR_GLXWINDOW_GLX_1_3_REQUIRED;
    using fabric::ERROR_GLXWINDOW_NO_FBCONFIG;
    using fabric::ERROR_GLXWINDOW_NO_VISUAL;
    using fabric::ERROR_GLXWINDOW_CREATEPBUFFER_FAILED;
    using fabric::ERROR_GLXWINDOW_FBCONFIG_REQUIRED;
    using fabric::ERROR_WGLWINDOW_NO_DRAWABLE;
    using fabric::ERROR_WGLWINDOW_SETPIXELFORMAT_FAILED;
    using fabric::ERROR_WGLWINDOW_REGISTERCLASS_FAILED;
    using fabric::ERROR_WGLWINDOW_FULLSCREEN_FAILED;
    using fabric::ERROR_WGLWINDOW_CREATEWINDOW_FAILED;
    using fabric::ERROR_WGLWINDOW_ARB_PBUFFER_REQUIRED;
    using fabric::ERROR_WGLWINDOW_CREATEPBUFFER_FAILED;
    using fabric::ERROR_WGLWINDOW_SETAFFINITY_PF_FAILED;
    using fabric::ERROR_WGLWINDOW_CHOOSE_PF_ARB_FAILED;
    using fabric::ERROR_WGLWINDOW_CREATECONTEXT_FAILED;
    using fabric::ERROR_CHANNEL_WINDOW_NOTRUNNING;
    using fabric::ERROR_PBO_UNSUPPORTED;
    using fabric::ERROR_PBO_NOT_INITIALIZED;
    using fabric::ERROR_PBO_SIZE_TOO_SMALL;
    using fabric::ERROR_PBO_TYPE_UNSUPPORTED;
    using fabric::ERROR_CUSTOM;
}
#endif // EQ_ERROR_H
