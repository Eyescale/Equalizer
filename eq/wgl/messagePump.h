
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_WGL_MESSAGEPUMP_H
#define EQ_WGL_MESSAGEPUMP_H

#include <eq/messagePump.h> // base class

namespace eq
{
namespace wgl
{
    /** Processes OS messages on Win32 systems. */
  class MessagePump : public eq::MessagePump
    {
    public:
        /** Construct a new WGL message pump. @version 1.0 */
        EQ_API MessagePump();

        /** Destruct this message pump. @version 1.0 */
        virtual ~MessagePump() {}

        EQ_API void postWakeup() override;
        EQ_API void dispatchAll() override;
        EQ_API void dispatchOne( const uint32_t timeout =
                                   LB_TIMEOUT_INDEFINITE ) override;

    private:
        /** Thread ID of the receiver. */
        unsigned long _win32ThreadID;
        
        void _initReceiverQueue();
    };
}
}
#endif //EQ_WGL_MESSAGEPUMP_H
