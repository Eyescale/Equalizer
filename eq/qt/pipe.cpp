
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

#include "pipe.h"

#include <eq/pipe.h>
#include <QDesktopWidget>

namespace eq
{
namespace qt
{
bool Pipe::configInit()
{
    eq::Pipe* pipe = getPipe();
    if( pipe->getPixelViewport().isValid( ))
        return true;

    QDesktopWidget desktop;
    const uint32_t device = getPipe()->getDevice();
    const int qtScreen = device == LB_UNDEFINED_UINT32 ?
                             desktop.primaryScreen() : int( device );
    const QRect rect = desktop.availableGeometry( qtScreen );

    pipe->setPixelViewport( PixelViewport( rect.x(), rect.y(),
                                           rect.width(), rect.height( )));
    return true;
}

}
}
