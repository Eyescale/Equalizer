
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

#ifndef EQBASE_THREADID_H
#define EQBASE_THREADID_H

#include <eq/base/base.h>     // EQ_EXPORT definition

namespace eq
{
namespace base
{
    class ThreadIDPrivate;

    /** An utility class to wrap OS-specific thread identifiers. */
    class ThreadID
    {
    public:
        /** Construct a new, zero thread identifier. @version 1.0 */
        EQ_EXPORT ThreadID();

        /** Construct a copy of a thread identifier. @version 1.0 */
        EQ_EXPORT ThreadID( const ThreadID& from );

        /** Destruct this thread identifier. @version 1.0 */
        EQ_EXPORT ~ThreadID();

        /** Assign another thread identifier. @version 1.0 */
        EQ_EXPORT ThreadID& operator = ( const ThreadID& from );

        /** @return true if the threads are equal, false if not. @version 1.0 */
        EQ_EXPORT bool operator == ( const ThreadID& rhs ) const;

        /**
         * @return true if the threads are different, false otherwise.
         * @version 1.0
         */
        EQ_EXPORT bool operator != ( const ThreadID& rhs ) const;
        
        EQ_EXPORT static const ThreadID ZERO; //!< a 'NULL' thread identifier

    private:
        ThreadIDPrivate* const _data;
        friend class Thread;

        friend std::ostream& operator << ( std::ostream& os, const ThreadID& );
    };

    /** Print the thread to the given output stream. */
    std::ostream& operator << ( std::ostream& os, const ThreadID& threadID );
}
}
#endif // EQBASE_THREADID_H

#ifdef PTHREAD_MUTEX_INITIALIZER // Crude test if pthread.h was included
#  ifndef EQBASE_THREADIDPRIVATE_H
#  define EQBASE_THREADIDPRIVATE_H

namespace eq
{
namespace base
{

class ThreadIDPrivate
{
public:
    pthread_t pthread;
};

}
}
#  endif // EQBASE_THREADIDPRIVATE_H
#endif // PTHREAD_MUTEX_INITIALIZER
