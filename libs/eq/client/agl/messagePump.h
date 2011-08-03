
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/client/agl/types.h>
#include <eq/client/messagePump.h>  // base class

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

        virtual void postWakeup();
        virtual void dispatchAll();
        virtual void dispatchOne();

    private:
        EventQueueRef _receiverQueue;
        EventRef      _wakeupEvent;
        bool          _needGlobalLock;

        struct Private;
        Private* _private; // placeholder for binary-compatible changes

        void _initReceiverQueue();
    };
}
}
#endif //EQ_AGL_MESSAGEPUMP_H
