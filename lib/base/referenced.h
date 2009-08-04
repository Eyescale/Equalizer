
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_REFERENCED_H
#define EQBASE_REFERENCED_H

#include <eq/base/base.h>     // for EQ_EXPORT
#include <eq/base/debug.h>    // for EQERROR
#include <eq/base/atomic.h>   // member
#include <typeinfo>

namespace eq
{
namespace base
{
    /**
     * Base class for referenced objects.
     * 
     * Implements reference-counted objects which destroy themselves once they
     * are no longer referenced. Uses an Atomic variable to keep the reference
     * count access thread-safe and efficient.
     *
     * @sa RefPtr
     */
    class Referenced 
    {
    public:
        /** Increase the reference count. */
        void ref()   
        {
#ifndef NDEBUG
            EQASSERTINFO( !_hasBeenDeleted, typeid( *this ).name( ));
#endif
            ++_refCount;
        }

        /** 
         * Decrease the reference count and delete this object when the
         * reference count reaches 0.
         */
        void unref() 
            { 
#ifndef NDEBUG
                EQASSERT( !_hasBeenDeleted );
#endif
                EQASSERT( _refCount > 0 ); 
                const bool deleteMe = (--_refCount==0);
                if( deleteMe )
                    deleteReferenced( this );
            }

        /** @return the current reference count. */
        int  getRefCount() const { return _refCount; }

    protected:
        /** Construct a new reference-counted object. */
        Referenced()
            : _refCount(0)
#ifndef NDEBUG
            , _hasBeenDeleted( false )
#endif
            {}

        /** Construct a new copy of a reference-counted object. */
        Referenced( const Referenced& ) 
            : _refCount(0)
#ifndef NDEBUG
            , _hasBeenDeleted( false )
#endif
            {}

        /** Destruct a reference-counted object. */
        virtual ~Referenced() 
            {
#ifndef NDEBUG
                EQASSERT( !_hasBeenDeleted );
                _hasBeenDeleted = true;
#endif
                EQASSERTINFO( _refCount == 0,
                              "Deleting object with ref count " << _refCount );
            }

    protected:
        EQ_EXPORT void deleteReferenced( Referenced* object );

    private:
        mtLong _refCount;
        bool _hasBeenDeleted;
    };
}

}
#endif //EQBASE_REFERENCED_H
