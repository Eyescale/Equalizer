
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef COBASE_CONDITION_H
#define COBASE_CONDITION_H

#include <co/base/os.h>
#include <co/base/debug.h>

namespace co
{
namespace base
{
    class ConditionPrivate;

    /**
     * A condition variable and associated lock.
     * Follows closely pthread_condition and mutex
     */
    class Condition
    {
    public:
        /** Construct a new condition variable. @version 1.0 */
        COBASE_API Condition();

        /** Destruct this condition variable. @version 1.0 */
        COBASE_API ~Condition();

        /** Lock the mutex. @version 1.0 */
        COBASE_API void lock();

        /** Signal the condition. @version 1.0 */
        COBASE_API void signal();

        /** Broadcast the condition. @version 1.0 */
        COBASE_API void broadcast();

        /** Unlock the mutex. @version 1.0 */
        COBASE_API void unlock();

        /**
         * Atomically unlock the mutex, wait for a signal and relock the mutex.
         * @version 1.0
         */
        COBASE_API void wait();

    private:
        ConditionPrivate* const _data;
    };
}
}

#endif //COBASE_CONDITION_H
