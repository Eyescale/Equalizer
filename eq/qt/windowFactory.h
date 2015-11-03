
/* Copyright (c) 2015, Stefan.Eilemann@epfl.ch
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

#ifndef EQ_QT_WINDOWSYSTEMFACTORY_H
#define EQ_QT_WINDOWSYSTEMFACTORY_H

#include <eq/types.h>
#include <QObject>

namespace eq
{
namespace qt
{
namespace detail { class Window; }

class WindowFactory : public QObject
{
    Q_OBJECT

public slots:
    detail::Window* onCreateImpl( const eq::Pipe*, const WindowSettings&,
                                  QThread* );
    void onDestroyImpl( detail::Window* window );
};

}
}

#endif
