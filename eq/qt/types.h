
/* Copyright (c) 2014-2016, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_QT_TYPES_H
#define EQ_QT_TYPES_H

#include <eq/types.h>

class QEvent;
class QExposeEvent;
class QHideEvent;
class QKeyEvent;
class QMouseEvent;
class QObject;
class QOpenGLContext;
class QResizeEvent;
class QThread;
class QWheelEvent;

namespace eq
{
/**
 * @namespace eq::qt
 * @brief The system abstraction layer for Qt
 */
namespace qt
{
class Event;
class EventHandler;
class Pipe;
class Window;
class WindowFactory;
class WindowIF;

namespace detail
{
class Window;
}
}
}

#endif // EQ_QT_TYPES_H
