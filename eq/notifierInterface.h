
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

#ifndef EQ_NOTIFIER_INTERFACE_H
#define EQ_NOTIFIER_INTERFACE_H

#include <eq/types.h>

namespace eq
{

/** A base class for notifying errors and events. */
class NotifierInterface
{
public:
    virtual ~NotifierInterface() {}

    /**
     * Send an error event to the application node.
     *
     * @param error the error code.
     * @version 1.7.2
     */
    virtual EventOCommand sendError( const uint32_t error ) = 0;

    /**
     * Process a received event.
     *
     * The task of this method is to update this object as necessary, and
     * transform the event into an config event to be send to the
     * application using Config::sendEvent().
     *
     * @param event the received event.
     * @return true when the event was handled, false if not.
     * @version 1.7.2
     */
    virtual bool processEvent( const Event& event ) = 0;
};
}


#endif // EQ_NOTIFIER_INTERFACE_H
