
/* Copyright (c) 2014, Daniel Nachbaur <danielnachbaur@gmail.com>
 *               2014, Stefan.Eilemann@epfl.ch
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

#ifndef EQ_QT_WIDGETFACTORY_H
#define EQ_QT_WIDGETFACTORY_H

#include <QObject>
#include <eq/client/types.h>
#include "types.h"

namespace eq
{
namespace qt
{

/** Creates and destroys the eq::qt::GLWidget in the QApplication thread. */
class WidgetFactory : public QObject
{
    Q_OBJECT

public slots:
    GLWidget* onCreateWidget( eq::Window*, const WindowSettings&, QThread* );
    void onDestroyWidget( GLWidget* );
};
}
}

#endif
