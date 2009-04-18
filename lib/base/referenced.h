
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

namespace eq
{
namespace base
{
    /**
     * Base class for referenced objects.
     * @sa RefPtr
     */
    class EQ_EXPORT Referenced 
    {
    public:
        // TODO: optional thread-safety
        void ref()   
        {
#ifndef NDEBUG
            EQASSERTINFO( !_hasBeenDeleted, typeid( *this ).name( ));
#endif
            ++_refCount;
        }
        void unref() 
            { 
#ifndef NDEBUG
                EQASSERT( !_hasBeenDeleted );
#endif
                EQASSERT( _refCount > 0 ); 
                const bool deleteMe = (--_refCount==0);
                if( deleteMe )
                    delete this;
            }

        int  getRefCount() const { return _refCount; }

    protected:
        Referenced()
            : _refCount(0)
#ifndef NDEBUG
            , _hasBeenDeleted( false )
#endif
            {}

        Referenced( const Referenced& ) 
            : _refCount(0)
#ifndef NDEBUG
            , _hasBeenDeleted( false )
#endif
            {}

        virtual ~Referenced() 
            {
#ifndef NDEBUG
                _hasBeenDeleted = true;
#endif
                if( _refCount!=0 ) 
                {
                    EQERROR << "Deleting object with ref count " << _refCount
                            << std::endl;
                }
                EQASSERT( _refCount == 0 );
            }

        mtLong _refCount;
#ifndef NDEBUG
        bool _hasBeenDeleted;
#endif
    };
}

}
#endif //EQBASE_REFERENCED_H
