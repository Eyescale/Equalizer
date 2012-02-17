
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_REFERENCED_H
#define COBASE_REFERENCED_H

#include <co/base/api.h>      // for COBASE_API
#include <co/base/atomic.h>   // member
#include <co/base/debug.h>    // for EQERROR
#include <co/base/refPtr.h>   // CO_REFERENCED_ARGS

#ifdef CO_REFERENCED_DEBUG
#  include <co/base/hash.h>
#  include <co/base/lock.h>
#  include <co/base/lockable.h>
#  include <co/base/scopedMutex.h>
#endif

namespace co
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
        /** Increase the reference count. @version 1.0 .*/
        void ref( CO_REFERENCED_ARGS ) const
        {
#ifndef NDEBUG
            EQASSERT( !_hasBeenDeleted );
#endif
            ++_refCount;

#ifdef CO_REFERENCED_DEBUG
            if( holder )
            {
                std::stringstream cs;
                cs << backtrace;
                ScopedMutex<> referencedMutex( _holders );
                HolderHash::iterator i = _holders->find( holder );
                EQASSERTINFO( i == _holders->end(), i->second );
                _holders.data[ holder ] = cs.str();
            }
#endif
        }

        /** 
         * Decrease the reference count.
         *
         * The object is deleted when the reference count reaches 0.
         * @version 1.0
         */
        void unref( CO_REFERENCED_ARGS ) const
            { 
#ifndef NDEBUG
                EQASSERT( !_hasBeenDeleted );
#endif
                EQASSERT( _refCount > 0 ); 
                const bool deleteMe = (--_refCount==0);
                if( deleteMe )
                    deleteReferenced( this );
#ifdef CO_REFERENCED_DEBUG
                else if( holder )
                {
                    ScopedMutex<> referencedMutex( _holders );
                    HolderHash::iterator i = _holders->find( holder );
                    EQASSERT( i != _holders->end( ));
                    _holders->erase( i );
                }
#endif
            }

        /** @return the current reference count. @version 1.0 */
        int  getRefCount() const { return _refCount; }

        /** @internal print holders of this if debugging is enabled. */
        void printHolders( std::ostream& os ) const
            {
#ifdef CO_REFERENCED_DEBUG
                os << disableFlush << disableHeader;
                ScopedMutex<> referencedMutex( _holders );
                for( HolderHash::const_iterator i = _holders->begin();
                     i != _holders->end(); ++i )
                {
                    os << "Holder " << i->first << ": " << i->second 
                       << std::endl;
                }
                os << enableHeader << enableFlush;
#endif
            }

    protected:
        /** Construct a new reference-counted object. @version 1.0 */
        Referenced()
                : _refCount( 0 )
                , _hasBeenDeleted( false )
            {}

        /** Construct a new copy of a reference-counted object. @version 1.0 */
        Referenced( const Referenced& ) 
                : _refCount( 0 )
                , _hasBeenDeleted( false )
            {}

        /** Destruct a reference-counted object. @version 1.0 */
        virtual ~Referenced() 
            {
#ifndef NDEBUG
                EQASSERT( !_hasBeenDeleted );
                _hasBeenDeleted = true;
#endif
                EQASSERTINFO( _refCount == 0,
                              "Deleting object with ref count " << _refCount );
            }

        /** Assign another object to this object. @version 1.1.3 */
        Referenced& operator = ( const Referenced& rhs ) { return *this; }

        COBASE_API void deleteReferenced( const Referenced* object ) const;

    private:
        mutable a_int32_t _refCount;
        bool _hasBeenDeleted;

#ifdef CO_REFERENCED_DEBUG
        typedef PtrHash< const void*, std::string > HolderHash;
        mutable Lockable< HolderHash, Lock > _holders;
#endif
    };
}
}

namespace boost
{
#ifdef CO_REFERENCED_DEBUG
#  define CO_BOOSTREF_ARGS 0
#  define CO_BOOSTREF_PARAM 0
#else
#  define CO_BOOSTREF_ARGS
#  define CO_BOOSTREF_PARAM
#endif

    /** Allow creation of boost::intrusive_ptr from RefPtr or Referenced. */
    inline void intrusive_ptr_add_ref( co::base::Referenced* referenced )
    {
        referenced->ref( CO_BOOSTREF_PARAM );
    }

    /** Allow creation of boost::intrusive_ptr from RefPtr or Referenced. */
    inline void intrusive_ptr_release( co::base::Referenced* referenced )
    {
        referenced->unref( CO_BOOSTREF_PARAM );
    }
}

#endif //COBASE_REFERENCED_H
