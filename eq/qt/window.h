
/* Copyright (c) 2014-2015, Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Stefan.Eilemann@epfl.ch
 *                          Juan Hernando <jhernando@fi.upm.es>
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

#include <eq/qt/types.h>
#include <eq/glWindow.h> // base class
#include <QObject> // base class
#include <QScreen>

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
    virtual ~WindowIF() {}

#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Woverloaded-virtual"
    /** Process the given event. @version 1.7.3 */
    virtual bool processEvent( const WindowEvent& event ) = 0;
#  pragma clang diagnostic pop
};

/** Equalizer default implementation of a Qt window */
class Window : public QObject, public WindowIF
{
    Q_OBJECT

public:
    /**
     * Create a new window using Qt
     *
     * The actual window will be a QWindow or a QOffscreenSurface depending
     * on the window settings.
     * The window won't be realized until configInit is called.
     *
     * @param parent The eq::Window parent window interface that uses this
     *        system window.
     * @param settings The window settings. The GL context format will be
     *        derived from these.
     * @param impl The Qt implementation (created in the app thread).
     *
     * @version 1.9
     */
    EQ_API Window( NotifierInterface& parent, const WindowSettings& settings,
                   detail::Window* impl );

    /** Destruct this Qt window. @version 1.7.3 */
    EQ_API ~Window() final;

    /** @name Qt initialization */
    //@{
    /**
     * Initialize this window for the Qt window system.
     *
     * The window will be only usable by the thread that invokes this
     * functions. Otherwise Qt thread affinity constraints will be violated.
     *
     * @return true if the initialization was successful, false otherwise.
     * @version 1.7.3
     */
    EQ_API bool configInit() override;

    /** @version 1.7.3 */
    EQ_API void configExit() override;

    //@}

    /**
     * The context won't be ready to be used until configInit is called.
     *
     * @return the Open GL context used by this window.
     * @version 1.9
     */
    EQ_API QOpenGLContext* getContext() const;

    /** @name Operations. */
    //@{
    /** @version 1.7.3 */
    EQ_API void makeCurrent( const bool cache = true ) const override;

    /** @version 1.10 */
    EQ_API void doneCurrent() const override;

    /** @version 1.7.3 */
    EQ_API void swapBuffers() override;

    /** Implementation untested for Qt. @version 1.7.3 */
    EQ_API void joinNVSwapBarrier( const uint32_t group,
                                   const uint32_t barrier ) override;

    /** Implementation untested for Qt. @version 1.7.3 */
    EQ_API void leaveNVSwapBarrier();

    /** @version 1.7.3 */
    EQ_API bool processEvent( const WindowEvent& event ) override;
    //@}

    /**
     * Use this object to make Qt events reach eq::Config when using this window
     * for offscreen rendering with shared context mode (e.g. to embed Equalizer
     * output into a Qt GUI).
     *
     * Don't send events directly to the object unless you know what you're
     * doing, use QApplication::postEvent instead.
     *
     * @return the object to which forward Qt events.
     * @version 1.9
     */
    EQ_API QObject* getEventProcessor();

    /** Move OpenGLContext object to given thread. @version 1.10 */
    void moveContextToThread( QThread* thread );

signals:
    void destroyImpl( detail::Window* );

private:
    detail::Window* const _impl;
    static detail::Window* createImpl( const WindowSettings&,
                                       QScreen*, QThread* );
    friend class WindowFactory;
};
}
}
#endif // EQ_QT_WINDOW_H
