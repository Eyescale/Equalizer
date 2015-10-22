
/* Copyright (c) 2007-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_AGL_MESSAGEPUMP_H
#define EQ_AGL_MESSAGEPUMP_H

#include <eq/defines.h>
#ifdef AGL

#include <eq/agl/types.h>
#include <eq/messagePump.h>  // base class
#include <lunchbox/clock.h> // member
namespace eq
{
namespace agl
{
    /** A message pump receiving and dispatching Carbon events. */
class MessagePump : public eq::MessagePump
    {
    public:
        /** Construct a new AGL message pump. @version 1.0 */
        MessagePump();

        /** Destruct this message pump. @version 1.0 */
        virtual ~MessagePump();

        void postWakeup() final;
        void dispatchAll() final;
        void dispatchOne( const uint32_t timeout=LB_TIMEOUT_INDEFINITE ) final;

    private:
        EventQueueRef _receiverQueue;
        EventRef      _wakeupEvent;
        lunchbox::Clock _clock;
        bool          _needGlobalLock;

        struct Private;
        Private* _private; // placeholder for binary-compatible changes

        void _initReceiverQueue();
    };
}
}
#endif // AGL
#endif // EQ_AGL_MESSAGEPUMP_H
