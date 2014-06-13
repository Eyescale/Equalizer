
/* Copyright (c) 2014, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_QT_WINDOW_H
#define EQ_QT_WINDOW_H

#include <eq/client/qt/types.h>
#include <eq/client/glWindow.h>       // base class

namespace eq
{
namespace qt
{
namespace detail { class Window; }

/** The interface defining the minimum functionality for a Qt window. */
class WindowIF : public GLWindow
{
public:
    WindowIF( NotifierInterface& parent,
              const WindowSettings& settings ) : GLWindow( parent, settings ) {}
    ~WindowIF() override {}

#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Woverloaded-virtual"
    /** Process the given event. @version 1.7.3 */
    virtual bool processEvent( const WindowEvent& event ) = 0;
#  pragma clang diagnostic pop
};

/** Equalizer default implementation of a Qt window */
class Window : public WindowIF
{
public:
    /**
     * Construct a new Qt system window.
     * @version 1.7.3
     */
    Window( NotifierInterface& parent, const WindowSettings& settings,
            GLWidget* glWidget );

    /** Destruct this Qt window. @version 1.7.3 */
    ~Window() final;

    /** @name Qt initialization */
    //@{
    /**
     * Initialize this window for the Qt window system.
     *
     * @return true if the initialization was successful, false otherwise.
     * @version 1.7.3
     */
    bool configInit() override;

    /** @version 1.7.3 */
    void configExit() override;

    /**
     * @version 1.7.3
     */
    EQ_API virtual void initEventHandler();

    /**
     * @version 1.7.3
     */
    EQ_API virtual void exitEventHandler();
    //@}

    /** @name Operations. */
    //@{
    /** @version 1.7.3 */
    void makeCurrent( const bool cache = true ) const override;

    /** @version 1.7.3 */
    void swapBuffers() override;

    /** Implementation untested for Qt. @version 1.7.3 */
    void joinNVSwapBarrier( const uint32_t group,
                            const uint32_t barrier ) override;

    /** Implementation untested for Qt. @version 1.7.3 */
    void leaveNVSwapBarrier();

    GLWidget* getGLWidget() const;

    /** @version 1.7.3 */
    EQ_API bool processEvent( const WindowEvent& event ) override;
    //@}

private:
    detail::Window* const _impl;
};
}
}
#endif // EQ_QT_WINDOW_H
