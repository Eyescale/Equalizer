
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_EXECUTIONLISTENER_H
#define EQBASE_EXECUTIONLISTENER_H

#include <eq/base/base.h>

namespace eq
{
namespace base
{
    /**
     * A listener interface to monitor execution unit (Thread, Process) state
     * changes.
     */
    class EQ_EXPORT ExecutionListener
    {
    public:
        /** Destruct the execution listener. @version 1.0 */
        virtual ~ExecutionListener() {}

        /** Notify that a new execution unit started. @version 1.0 */
        virtual void notifyExecutionStarted() {}

        /** Notify that the execution unit is about to stop. @version 1.0 */
        virtual void notifyExecutionStopping() {}
    };
}

}
#endif //EQBASE_EXECUTIONLISTENER_H
