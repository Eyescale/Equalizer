
/* Copyright (c) 2009-2015, Stefan.Eilemann@epfl.ch
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

#ifndef EQ_X11_WINDOW_H
#define EQ_X11_WINDOW_H

#include <eq/systemWindow.h>         // base class
#include <eq/glx/types.h>

namespace eq
{
/**
 * @namespace eq::x11
 * @brief An abstraction layer for CPU-based rendering using X11.
 */
namespace x11
{

/**
 * A system window for CPU rendering on X11.
 *
 * Example usage: @include examples/eqCPU/window.cpp
 * @version 1.9
 */
class Window : public SystemWindow
{
public:
    Window( NotifierInterface& parent, const WindowSettings& settings,
            Display* xDisplay );

    bool configInit() override;
    void configExit() override;
    void makeCurrent( bool /*cache*/ ) const override {}
    void doneCurrent() const override {}
    void bindFrameBuffer() const override {}
    void bindDrawFrameBuffer() const override {}
    void updateFrameBuffer() const override {}
    void swapBuffers() override {}
    void joinNVSwapBarrier( const uint32_t, const uint32_t ) override {}
    void queryDrawableConfig( eq::DrawableConfig& drawableConfig ) override;
    void flush() override;
    void finish() override { flush(); }

    /** @return the X11 display connection */
    virtual Display* getXDisplay();

    /**  @return the X11 drawable ID. */
    virtual XID getXDrawable() const { return _xDrawable; }

    /** @name Data Access */
    //@{
    /**
     * Set the X11 drawable ID for this window.
     *
     * This function should only be called from configInit() or
     * configExit().
     *
     * @param drawable the X11 drawable ID.
     */
    virtual void setXDrawable( XID drawable ) { _xDrawable = drawable; }

private:
    Window( const Window& ) = delete;
    Window& operator=( const Window& ) = delete;
    XID _createWindow();
    Display* _xDisplay; //!< The display connection (maintained by GLXPipe)
    XID _xDrawable;
};

}
}

#endif // EQ_X11_WINDOW_H
