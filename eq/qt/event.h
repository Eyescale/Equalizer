
/* Copyright (c) 2014-2016, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_QT_EVENT_H
#define EQ_QT_EVENT_H

#include <QEvent> // base class
#include <eq/api.h>
#include <eq/types.h>

namespace eq
{
namespace qt
{
/** A window-system event for a qt::WindowIF. */
class EQ_API Event : public QEvent
{
public:
    Event(const QEvent* from)
        : QEvent(QEvent::User)
        , qtype(from->type())
    {
        static_cast<QEvent&>(*this) = *from;
    }

    const int32_t qtype;
};
}
}
#endif
