
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_WGLMESSAGEPUMP_H
#define EQ_WGLMESSAGEPUMP_H

#include <eq/messagePump.h> // base class

namespace eq
{
    /** Processes OS messages on Win32 systems. */
    class WGLMessagePump : public MessagePump
    {
    public:
        /** Construct a new WGL message pump. @version 1.0 */
        EQ_API WGLMessagePump();

        /** Destruct this message pump. @version 1.0 */
        virtual ~WGLMessagePump() {}

        EQ_API virtual void postWakeup();
        EQ_API virtual void dispatchAll();
        EQ_API virtual void dispatchOne();

    private:
        /** Thread ID of the receiver. */
        DWORD _win32ThreadID;

        struct Private;
        Private* _private; // placeholder for binary-compatible changes

        void _initReceiverQueue();
    };
}

#endif //EQ_WGLMESSAGEPUMP_H
