
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

#ifndef EQ_QT_GLWIDGET_H
#define EQ_QT_GLWIDGET_H

#include <eq/client/qt/types.h>
#include <lunchbox/compiler.h> // override, final
#include <QGLWidget> // base class

namespace eq
{
namespace qt
{

class GLWidget : public QGLWidget
{
public:
    GLWidget( const QGLFormat& format, const GLWidget* shareWidget );
    ~GLWidget() final;

    void setParent( qt::Window* parent_ );

    void initEventHandler();
    void exitEventHandler();

    void paintEvent( QPaintEvent* ) override;
    void resizeEvent( QResizeEvent* ) override;
    void closeEvent( QCloseEvent* ) override;

    void mousePressEvent( QMouseEvent* ) override;
    void mouseReleaseEvent( QMouseEvent* ) override;
    void mouseMoveEvent( QMouseEvent* ) override;
#ifndef QT_NO_WHEELEVENT
    void wheelEvent( QWheelEvent* ) override;
#endif
    void keyPressEvent( QKeyEvent* ) override;
    void keyReleaseEvent( QKeyEvent* ) override;

private:
    qt::Window* _parent;
    EventHandler* _eventHandler;
};

}
}
#endif // EQ_QT_GLWIDGET_H
