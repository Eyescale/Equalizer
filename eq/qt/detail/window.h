
/* Copyright (c) 2014-2017, Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Juan Hernando <jhernando@fi.upm.es>
 *                          Stefan.Eilemann@epfl.ch
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

#ifndef EQ_QT_WINDOWIMPL_H
#define EQ_QT_WINDOWIMPL_H

#include "../eventHandler.h"

#include <eq/util/frameBufferObject.h>
#include <lunchbox/monitor.h>

#include <QApplication>
#include <QExposeEvent>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QWindow>

namespace eq
{
namespace qt
{
namespace detail
{
namespace
{
#define getAttribute(attr) settings.getIAttribute(WindowSettings::attr)

QSurfaceFormat _createFormat(const WindowSettings& settings)
{
    // defaults: http://doc.qt.io/qt-5/qsurfaceformat.html
    QSurfaceFormat format;

    const int coreProfile = getAttribute(IATTR_HINT_CORE_PROFILE);
    if (coreProfile == ON)
    {
        format.setMajorVersion(getAttribute(IATTR_HINT_OPENGL_MAJOR));
        format.setMinorVersion(getAttribute(IATTR_HINT_OPENGL_MINOR));
        format.setProfile(QSurfaceFormat::CoreProfile);
    }

    const int colorSize = getAttribute(IATTR_PLANES_COLOR);
    if (colorSize > 0 || colorSize == eq::AUTO)
    {
        const int colorBits = (colorSize > 0 ? colorSize : 8);
        format.setRedBufferSize(colorBits);
        format.setGreenBufferSize(colorBits);
        format.setBlueBufferSize(colorBits);
    }

    const int alphaPlanes = getAttribute(IATTR_PLANES_ALPHA);
    if (alphaPlanes > 0 || alphaPlanes == eq::AUTO)
    {
        const int alphaBits = (alphaPlanes > 0 ? alphaPlanes : 8);
        format.setAlphaBufferSize(alphaBits);
    }

    const int depthPlanes = getAttribute(IATTR_PLANES_DEPTH);
    if (depthPlanes > 0 || depthPlanes == eq::AUTO)
    {
        const int depthBits = (depthPlanes > 0 ? depthPlanes : 8);
        format.setDepthBufferSize(depthBits);
    }
    else
        format.setDepthBufferSize(0);

    const int stencilPlanes = getAttribute(IATTR_PLANES_STENCIL);
    if (stencilPlanes > 0 || stencilPlanes == eq::AUTO)
    {
        const int stencilBits = (stencilPlanes > 0 ? stencilPlanes : 8);
        format.setStencilBufferSize(stencilBits);
    }
    else
        format.setStencilBufferSize(0);

    const int samplesPlanes = getAttribute(IATTR_PLANES_SAMPLES);
    if (samplesPlanes >= 0)
    {
        format.setSamples(samplesPlanes);
    }

    if (getAttribute(IATTR_HINT_STEREO) == eq::ON)
    {
        format.setStereo(true);
    }

    if (getAttribute(IATTR_HINT_DOUBLEBUFFER) == eq::ON ||
        (getAttribute(IATTR_HINT_DOUBLEBUFFER) == eq::AUTO &&
         getAttribute(IATTR_HINT_DRAWABLE) == eq::WINDOW))
    {
        format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    }
    else
        format.setSwapBehavior(QSurfaceFormat::SingleBuffer);

    return format;
}
#undef getAttribute
}

class Window
{
public:
    Window(WindowIF& window, QThread* qThread)
        : _eventHandler(window)
    {
        if (qThread)
            _eventHandler.moveToThread(qThread);
    }

    virtual ~Window() {}
    virtual QOpenGLContext* getContext() const = 0;
    virtual bool configInit() = 0;
    virtual void makeCurrent() = 0;
    virtual void doneCurrent() = 0;
    virtual void swapBuffers() = 0;
    virtual void resize(const PixelViewport&){};
    virtual QObject* getEventProcessor() { return &_eventHandler; }
protected:
    EventHandler _eventHandler;
};

class QWindowWrapper : public Window, public QWindow
{
public:
    QWindowWrapper(WindowIF& window, QThread* qThread,
                   const WindowSettings& settings, QScreen* screen_,
                   QOpenGLContext* shareContext)
        : detail::Window(window, qThread)
        , _context(new QOpenGLContext)
        , _realized(false)
    {
        setScreen(screen_);
        const QSurfaceFormat& format_ = _createFormat(settings);
        setFormat(format_);
        const PixelViewport& pvp = settings.getPixelViewport();
        setGeometry(pvp.x, pvp.y, pvp.w, pvp.h);
        setSurfaceType(QSurface::OpenGLSurface);

        _context->setFormat(format());
        _context->setShareContext(shareContext);

        if (settings.getIAttribute(WindowSettings::IATTR_HINT_FULLSCREEN) ==
            eq::ON)
        {
            showFullScreen();
        }
        else
            show();
    }

    QOpenGLContext* getContext() const final { return _context.data(); }
    void makeCurrent() final
    {
        if (!_context->makeCurrent(this))
            LBWARN << "QOpenGLContext::makeCurrent failed: " << _context.data()
                   << std::endl;
    }

    void doneCurrent() final { _context->doneCurrent(); }
    void swapBuffers() final
    {
        if (isExposed())
            _context->swapBuffers(this);
    }

    void resize(const PixelViewport& pvp) final
    {
        setPosition(pvp.x, pvp.y);
        QWindow::resize(pvp.w, pvp.h);
    }

    bool configInit() final
    {
        // The application thread must be running the event loop somewhere
        // else. Otherwise this code will wait forever.
        _realized.waitEQ(true);

        if (!_context->create())
            return false;

        // Adjusting the actual window viewport in case the window manager
        // disobeyed the requested geometry.
        WindowIF& window = _eventHandler.getWindow();
        window.setPixelViewport(PixelViewport(x(), y(), width(), height()));
        return true;
    }

    QObject* getEventProcessor() final { return this; }
protected:
    bool event(QEvent* qevent) final
    {
        if (qevent->type() == QEvent::Expose)
        {
            const QExposeEvent* expose = static_cast<QExposeEvent*>(qevent);
            _realized = !expose->region().isEmpty();
        }

        QApplication::sendEvent(&_eventHandler, qevent);
        return true;
    }

private:
    QScopedPointer<QOpenGLContext> _context;

    lunchbox::Monitorb _realized;
};

class QOffscreenSurfaceWrapper : public Window, public QOffscreenSurface
{
public:
    QOffscreenSurfaceWrapper(WindowIF& window, QThread* qThread,
                             const WindowSettings& settings, QScreen* screen_,
                             QOpenGLContext* shareContext)
        : detail::Window(window, qThread)
        , _context(new QOpenGLContext)
    {
        setScreen(screen_);
        const QSurfaceFormat& format_ = _createFormat(settings);
        setFormat(format_);

        _context->setFormat(format());
        _context->setShareContext(shareContext);
        create();
    }

    QOpenGLContext* getContext() const final { return _context.data(); }
    void makeCurrent() final
    {
        if (!_context->makeCurrent(this))
            LBWARN << "QOpenGLContext::makeCurrent failed: " << _context.data()
                   << std::endl;
    }

    void doneCurrent() final { _context->doneCurrent(); }
    void swapBuffers() final
    {
        if (isValid())
            _context->swapBuffers(this);
    }

    bool configInit() final { return isValid() && _context->create(); }
private:
    QScopedPointer<QOpenGLContext> _context;
};
}
}
}

#endif
