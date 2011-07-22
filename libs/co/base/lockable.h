
/* Copyright (c) 2009-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_LOCKABLE_H
#define COBASE_LOCKABLE_H

#include <co/base/log.h> // used inline
#include <co/base/nonCopyable.h> // base class
#include <iostream>

namespace co
{
namespace base
{
    class Lock;

    /**
     * A convenience structure to hold data together with a lock for access.
     * 
     * Locking the data still has to be done manually, e.g, using a ScopedMutex.
     */
    template< typename D, class L = Lock > class Lockable : public NonCopyable
    {
    public:
        /** Construct a new lockable data structure. @version 1.0 */
        explicit Lockable() {}

        /** Construct and initialize a new data structure. @version 1.0 */
        explicit Lockable( const D& value ) : data( value ) {}

        /** Access the held data. @version 1.0 */
        D* operator->() { return &data; }

        /** Access the held data. @version 1.0 */
        const D* operator->() const { return &data; }

        /** @return true if the data is equal to the rhs object. @version 1.0*/ 
        bool operator == ( const D& rhs ) const { return ( data == rhs ); }

        /** Assign another value to the data. @version 1.0 */
        Lockable& operator = ( const D& rhs ) { data = rhs; return *this; }

        D data;
        mutable L lock;
    };

    /** Print the data to the given output stream. */
    template< class D, class L >
    inline std::ostream& operator << ( std::ostream& os,
                                       const Lockable<D, L>& l )
    {
        os << disableFlush << "<" << l.lock.isSet() << " " << l.data << ">"
           << enableFlush;
        return os;
    }
}
}
#endif // COBASE_LOCKABLE_H
