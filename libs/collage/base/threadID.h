
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

#include <co/base/os.h>     // EQBASE_API definition

namespace eq
{
namespace base
{
    struct ThreadIDPrivate;

    /** An utility class to wrap OS-specific thread identifiers. */
    class ThreadID
    {
    public:
        /** Construct a new, zero thread identifier. @version 1.0 */
        EQBASE_API ThreadID();

        /** Construct a copy of a thread identifier. @version 1.0 */
        EQBASE_API ThreadID( const ThreadID& from );

        /** Destruct this thread identifier. @version 1.0 */
        EQBASE_API ~ThreadID();

        /** Assign another thread identifier. @version 1.0 */
        EQBASE_API ThreadID& operator = ( const ThreadID& from );

        /** @return true if the threads are equal, false if not. @version 1.0 */
        EQBASE_API bool operator == ( const ThreadID& rhs ) const;

        /**
         * @return true if the threads are different, false otherwise.
         * @version 1.0
         */
        EQBASE_API bool operator != ( const ThreadID& rhs ) const;
        
        EQBASE_API static const ThreadID ZERO; //!< a 'NULL' thread identifier

    private:
        ThreadIDPrivate* const _data;
        friend class Thread;

        friend EQBASE_API 
        std::ostream& operator << ( std::ostream& os, const ThreadID& );
    };

    /** Print the thread to the given output stream. */
    EQBASE_API std::ostream& operator << ( std::ostream&, const ThreadID& );
}
}

#endif // EQBASE_THREADID_H
