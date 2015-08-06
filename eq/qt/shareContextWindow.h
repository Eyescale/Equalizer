
/* Copyright (c) 2015, Juan Hernando <jhernando@fi.upm.es>
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

#ifndef EQ_QT_SHARECONTEXTWINDOW_H
#define EQ_QT_SHARECONTEXTWINDOW_H

#include "types.h"

#include <eq/systemWindow.h> // base class

namespace eq
{
namespace qt
{

/**
   Helper window to allow Open GL context sharing between Qt widgets and
   windows created by Equalizer.

   This window is not intended to be used for any actual work except for
   being able to provide a share OpenGLContext to WindowSystem::createWindow
   through the WindowSettings.
*/
class ShareContextWindow : public eq::SystemWindow
{
public:
    ShareContextWindow( QOpenGLContext* context,
                        NotifierInterface& parent,
                        const WindowSettings& settings )
        : SystemWindow( parent, settings )
        , _context(context)
    {}


    QOpenGLContext* getContext() const
    {
        return _context;
    }

    bool configInit() final { LBUNIMPLEMENTED; return false; }
    void configExit() final { LBUNIMPLEMENTED }
    void makeCurrent( const bool ) const final { LBUNIMPLEMENTED }
    void doneCurrent() const final { LBUNIMPLEMENTED }
    void bindFrameBuffer() const final { LBUNIMPLEMENTED }
    void bindDrawFrameBuffer() const final { LBUNIMPLEMENTED }
    void updateFrameBuffer() const final { LBUNIMPLEMENTED }
    void swapBuffers() final { LBUNIMPLEMENTED }
    void flush() final { LBUNIMPLEMENTED }
    void finish() final { LBUNIMPLEMENTED }
    void joinNVSwapBarrier( const uint32_t, const uint32_t ) final
        { LBUNIMPLEMENTED }
    void queryDrawableConfig( DrawableConfig& ) final { LBUNIMPLEMENTED }
    bool processEvent( const Event& ) final { LBUNIMPLEMENTED; return false; }

private:
    QOpenGLContext* _context;
};

}
}
#endif // EQ_QT_SHARECONTEXTWINDOW_H
