
/* Copyright (c) 2010-2014, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQFABRIC_ERROR_H
#define EQFABRIC_ERROR_H

#include <eq/fabric/api.h>
#include <lunchbox/types.h>

namespace eq
{
namespace fabric
{
/** Defines errors produced by Equalizer classes. */
enum ErrorCode
{
    ERROR_NONE = 0,
    ERROR_FBO_UNSUPPORTED,
    ERROR_FRAMEBUFFER_STATUS,
    ERROR_FRAMEBUFFER_UNSUPPORTED,
    ERROR_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
    ERROR_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
    ERROR_FRAMEBUFFER_INCOMPLETE_DIMENSIONS,
    ERROR_FRAMEBUFFER_INCOMPLETE_FORMATS,
    ERROR_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
    ERROR_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
    ERROR_FRAMEBUFFER_INITIALIZED,
    ERROR_CUDACONTEXT_DEVICE_NOTFOUND,
    ERROR_CUDACONTEXT_INIT_FAILED,
    ERROR_CUDACONTEXT_MISSING_SUPPORT,
    ERROR_WINDOWSYSTEM_UNKNOWN,
    ERROR_NODE_LAUNCH,
    ERROR_NODE_CONNECT,
    ERROR_PIPE_NODE_NOTRUNNING,
    ERROR_SYSTEMPIPE_PIXELFORMAT_NOTFOUND,
    ERROR_SYSTEMPIPE_CREATECONTEXT_FAILED,
    ERROR_SYSTEMPIPE_CREATEWINDOW_FAILED,
    ERROR_AGLPIPE_DISPLAYS_NOTFOUND,
    ERROR_AGLPIPE_DEVICE_NOTFOUND,
    ERROR_GLXPIPE_DEVICE_NOTFOUND,
    ERROR_GLXPIPE_GLX_NOTFOUND,
    ERROR_GLXPIPE_GLXEWINIT_FAILED,
    ERROR_WGL_CREATEAFFINITYDC_FAILED,
    ERROR_WGLPIPE_ENUMDISPLAYS_FAILED,
    ERROR_WGLPIPE_CREATEDC_FAILED,
    ERROR_WGLPIPE_ENUMGPUS_FAILED,
    ERROR_WGLPIPE_REGISTERCLASS_FAILED,
    ERROR_WGLPIPE_SETPF_FAILED,
    ERROR_WGLPIPE_WGLEWINIT_FAILED,
    ERROR_WINDOW_PIPE_NOTRUNNING,
    ERROR_WINDOW_PVP_INVALID,
    ERROR_SYSTEMWINDOW_PIXELFORMAT_NOTFOUND,
    ERROR_SYSTEMWINDOW_NO_PIXELFORMAT,
    ERROR_SYSTEMWINDOW_ARB_FLOAT_FB_REQUIRED,
    ERROR_AGLWINDOW_NO_CONTEXT,
    ERROR_AGLWINDOW_CREATECONTEXT_FAILED,
    ERROR_AGLWINDOW_CREATEPBUFFER_FAILED,
    ERROR_AGLWINDOW_SETPBUFFER_FAILED,
    ERROR_AGLWINDOW_CREATEWINDOW_FAILED,
    ERROR_AGLWINDOW_SETWINDOW_FAILED,
    ERROR_GLXWINDOW_NO_DRAWABLE,
    ERROR_GLXWINDOW_NO_DISPLAY,
    ERROR_GLXWINDOW_CREATECONTEXT_FAILED,
    ERROR_GLXWINDOW_CREATEWINDOW_FAILED,
    ERROR_GLXWINDOW_GLXQUERYVERSION_FAILED,
    ERROR_GLXWINDOW_GLX_1_3_REQUIRED,
    ERROR_GLXWINDOW_NO_FBCONFIG,
    ERROR_GLXWINDOW_NO_VISUAL,
    ERROR_GLXWINDOW_CREATEPBUFFER_FAILED,
    ERROR_GLXWINDOW_FBCONFIG_REQUIRED,
    ERROR_WGLWINDOW_NO_DRAWABLE,
    ERROR_WGLWINDOW_SETPIXELFORMAT_FAILED,
    ERROR_WGLWINDOW_REGISTERCLASS_FAILED,
    ERROR_WGLWINDOW_FULLSCREEN_FAILED,
    ERROR_WGLWINDOW_CREATEWINDOW_FAILED,
    ERROR_WGLWINDOW_ARB_PBUFFER_REQUIRED,
    ERROR_WGLWINDOW_CREATEPBUFFER_FAILED,
    ERROR_WGLWINDOW_SETAFFINITY_PF_FAILED,
    ERROR_WGLWINDOW_CHOOSE_PF_ARB_FAILED,
    ERROR_WGLWINDOW_CREATECONTEXT_FAILED,
    ERROR_CHANNEL_WINDOW_NOTRUNNING,
    ERROR_PBO_UNSUPPORTED,
    ERROR_PBO_NOT_INITIALIZED,
    ERROR_PBO_SIZE_TOO_SMALL,
    ERROR_PBO_TYPE_UNSUPPORTED,

    ERROR_CUSTOM = LB_64KB, // 0x10000
};

/** A wrapper for error codes to allow intuitive bool-like usage. */
class Error
{
    typedef void (Error::*bool_t)() const;
    void bool_true() const {}

public:
    /** Construct a new error. @version 1.7.1 */
    explicit Error( const uint32_t code ) : code_( code ) {}

    /** Assign the given error code. @version 1.7.1*/
    Error& operator = ( const ErrorCode code ) { code_ = code; return *this; }

    /** @return true if no error occured. @version 1.7.1 */
    operator bool_t() { return code_ == ERROR_NONE ? &Error::bool_true : 0; }

    /** @return true if an error occured. @version 1.7.1 */
    bool operator ! () { return code_ != ERROR_NONE; }

    /** @return the error code. @version 1.7.1 */
    uint32_t getCode() const { return code_; }

    /** @return the error code. @version 1.7.1 */
    operator uint32_t() const { return code_; }

    /** @return true if the two errors have the same value. @version 1.7.1*/
    bool operator == ( const Error& rhs ) const { return code_ == rhs.code_; }

    /** @return true if the two errors have different values. @version 1.7.1*/
    bool operator != ( const Error& rhs ) const { return code_ != rhs.code_; }

    /** @return true if the two errors have the same value. @version 1.7.1*/
    bool operator == ( const uint32_t code ) const { return code_ == code; }

    /** @return true if the two errors have different values. @version 1.7.1*/
    bool operator != ( const uint32_t code ) const { return code_ != code; }

private:
    uint32_t code_;
};

/** Print the error in a human-readable format. @version 1.0 */
EQFABRIC_API std::ostream& operator << ( std::ostream& os, const Error& );

}
}
#endif // EQFABRIC_ERROR_H
