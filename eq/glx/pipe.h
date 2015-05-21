
/* Copyright (c) 2009-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2009, Maxim Makhinya
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

#ifndef EQ_GLX_PIPE_H
#define EQ_GLX_PIPE_H

#include <eq/client/glx/types.h>

#include <eq/client/systemPipe.h> // base class

namespace eq
{
namespace glx
{
namespace detail { class Pipe; }

/** Default implementation of a glX system pipe. */
class Pipe : public SystemPipe
{
public:
    /** Construct a new glX system pipe. @version 1.0 */
    Pipe( eq::Pipe* parent );

    /** Destruct this glX pipe. @version 1.0 */
    virtual ~Pipe();

    /** @name GLX/X11 initialization */
    //@{
    /**
     * Initialize this pipe for the GLX window system.
     *
     * @return true if the initialization was successful, false otherwise.
     * @version 1.0
     */
    EQ_API virtual bool configInit();

    /**
     * Deinitialize this pipe for the GLX window system.
     *
     * @return true if the deinitialization was successful, false otherwise.
     * @version 1.0
     */
    EQ_API virtual void configExit();
    //@}

    /** @return the X display connection for this pipe. @version 1.0 */
    EQ_API Display* getXDisplay() const;

    /** @return the generic GLX function table for the pipe. */
    EQ_API GLXEWContext* glxewGetContext();

    /** @internal @return the GPU info queried from an open Display. */
    static bool getGPUInfo( Display* display, GPUInfo& info );

protected:
    /**
     * Set the X display connection for this pipe.
     *
     * This function should only be called from configInit() or
     * configExit(). Updates the pixel viewport. Calls XSetCurrentDisplay().
     *
     * @param display the X display connection for this pipe.
     * @version 1.0
     */
    void setXDisplay( Display* display );

    /**
     * @return The string representation of this pipe's port and device
     *         setting, in the form used by XOpenDisplay().
     * @version 1.0
     */
    std::string getXDisplayString();

    /**
     * Initialize this pipe for OpenGL.
     *
     * A temporary GL context is current during this call. The context is
     * not the one used by the windows of this pipe.
     *
     * @version 1.0
     */
    virtual bool configInitGL() { return true; }

private:
    detail::Pipe* const _impl;

    bool _configInitGLXEW();
};
}
}
#endif // EQ_GLX_PIPE_H
