
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

#ifndef EQ_QT_MESSAGEPUMP_H
#define EQ_QT_MESSAGEPUMP_H

#include <eq/client/messagePump.h> // base class

namespace eq
{
namespace qt
{
/** A message pump receiving and dispatching Qt events. */
class MessagePump : public eq::MessagePump
{
public:
    /** Construct a new Qt message pump. @version 1.7.3 */
    MessagePump();

    /** Destruct this message pump. @version 1.7.3 */
    ~MessagePump() final;

    void postWakeup() override;
    void dispatchAll() override;
    void dispatchOne( const uint32_t timeout = LB_TIMEOUT_INDEFINITE ) override;
};
}
}
#endif // EQ_QT_MESSAGEPUMP_H
