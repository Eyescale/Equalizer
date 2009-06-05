
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_AGLMESSAGEPUMP_H
#define EQ_AGLMESSAGEPUMP_H

#include <eq/client/messagePump.h>  // base class
#include <eq/client/windowSystem.h> // EventQueueRef definition

namespace eq
{
    /**
     * Processes OS messages on AGL/Carbon.
     */
    class AGLMessagePump : public MessagePump
    {
    public:
        AGLMessagePump();

        /** Wake up dispatchOneEvent(). */
        virtual void postWakeup();

        /** Get and dispatch all pending system events, non-blocking. */
        virtual void dispatchAll();

        /** Get and dispatch at least one pending system event, blocking. */
        virtual void dispatchOne();

        virtual ~AGLMessagePump();

    private:
        EventQueueRef _receiverQueue;
        EventRef      _wakeupEvent;
        bool          _needGlobalLock;

        void _initReceiverQueue();
    };
}

#endif //EQ_AGLMESSAGEPUMP_H
