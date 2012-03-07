
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1q as published
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

#ifndef COBASE_REFPTR_H
#define COBASE_REFPTR_H

#include <co/base/debug.h>

#include <iostream>
#include <stdlib.h>

//#define CO_REFERENCED_DEBUG
#ifdef CO_REFERENCED_DEBUG
#  define CO_REFERENCED_ARGS const void* holder
#  define CO_REFERENCED_PARAM (const void*)(this)
#else
#  define CO_REFERENCED_ARGS
#  define CO_REFERENCED_PARAM
#endif

namespace co
{
namespace base
{
    /**
     * A smart reference pointer, aka boost::intrusive_ptr.
     *
     * Relies on the held object to implement ref() and unref() correctly.
     */
    template< class T > class RefPtr 
    {
        typedef T* RefPtr::*bool_t;

    public:
        /** Construct a new, empty reference pointer. @version 1.0 */
        RefPtr()                     : _ptr( 0 )         {}

        /** Construct a reference pointer from a C pointer. @version 1.0 */
        RefPtr( T* const ptr )       : _ptr( ptr )       { _ref(); }

        /** Construct a copy of a reference pointer. @version 1.0 */
        RefPtr( const RefPtr& from ) : _ptr( from._ptr ) { _ref(); }

        /**
         * Construct a copy of a reference pointer of a different type.
         * @version 1.0
         */
        template< class O > RefPtr( RefPtr< O > from )
                : _ptr( from.get( )) { _ref(); }

        /** Destruct this reference pointer. @version 1.0 */
        ~RefPtr() { _unref(); _ptr = 0; }

        /** Assign another RefPtr to this reference pointer. @version 1.0 */
        RefPtr& operator = ( const RefPtr& rhs )
            {
                if( _ptr == rhs._ptr )
                    return *this;

                T* tmp = _ptr;
                _ptr = rhs._ptr;
                _ref();
                if( tmp ) tmp->unref( CO_REFERENCED_PARAM );
                return *this;
            }

        /** Assign a C pointer to this reference pointer. @version 1.0 */
        RefPtr& operator = ( T* ptr )
            {
                if( _ptr == ptr )
                    return *this;

                T* tmp = _ptr;
                _ptr = ptr;
                _ref();
                if( tmp ) tmp->unref( CO_REFERENCED_PARAM );
                return *this;
            }

        /**
         * @return true if both reference pointers hold the same C pointer.
         * @version 1.0
         */
        bool operator == ( const RefPtr& rhs ) const 
            { return ( _ptr == rhs._ptr ); }

        /**
         * @return true if both reference pointer hold different C pointer.
         * @version 1.0
         */
        bool operator != ( const RefPtr& rhs ) const
            { return ( _ptr != rhs._ptr ); }

        /**
         * @return true if a pointer is held, false otherwise.
         * @version 1.1.5
         */
        operator bool_t() const { return _ptr == 0 ? 0 : &RefPtr::_ptr; }

        /**
         * @return true if the left RefPtr is smaller then the right.
         * @version 1.0
         */
        bool operator < ( const RefPtr& rhs ) const
            { return ( _ptr < rhs._ptr ); }

        /**
         * @return true if the right RefPtr is smaller then the left.
         * @version 1.0
         */
        bool operator > ( const RefPtr& rhs ) const
            { return ( _ptr > rhs._ptr ); }

        /** @return true if the RefPtr is empty. @version 1.0 */
        bool operator ! () const               { return ( _ptr==0 ); }

        /**
         * @return true if the reference pointers holds the C pointer.
         * @version 1.0
         */
        bool operator == ( const T* ptr ) const { return ( _ptr == ptr ); }

        /**
         * @return true if the reference pointers does not hold the C pointer
         * @version 1.0
         */
        bool operator != ( const T* ptr ) const { return ( _ptr != ptr ); }

        /** Access the held object. @version 1.0 */
        T*       operator->()       
            { EQASSERTINFO( _ptr, className( this )); return _ptr; }
        /** Access the held object. @version 1.0 */
        const T* operator->() const
            { EQASSERTINFO( _ptr, className( this )); return _ptr; }
        /** Access the held object. @version 1.0 */
        T&       operator*()        
            { EQASSERTINFO( _ptr, className( this )); return *_ptr; }
        /** Access the held object. @version 1.0 */
        const T& operator*() const  
            { EQASSERTINFO( _ptr, className( this )); return *_ptr; }

        /** @return the C pointer. @version 1.0 */
        T*       get()                { return _ptr; }
        /** @return the C pointer. @version 1.0 */
        const T* get() const          { return _ptr; }

        /** @return true if the RefPtr holds a non-0 pointer. @version 1.0 */
        bool isValid() const { return ( _ptr != 0 ); }
        
    private:
        T* _ptr;

        /** Artificially reference the held object. */
        void _ref()   { if(_ptr) _ptr->ref( CO_REFERENCED_PARAM ); }

        /** Artificially dereference the held object. */
        void _unref() 
        {
            if(_ptr)
            {
#ifndef NDEBUG
                const bool abondon = (_ptr->getRefCount() == 1);
                _ptr->unref( CO_REFERENCED_PARAM );
                if( abondon ) 
                    _ptr = 0;
#else
                _ptr->unref( CO_REFERENCED_PARAM );
#endif
            }
        }
    };

    /** Print the reference pointer to the given output stream. */
    template< class T >
    inline std::ostream& operator << ( std::ostream& os, const RefPtr<T>& rp )
    {
        const T* p = rp.get();
        if( p )
        {
            os << disableFlush << "RP[" << *p << "]";
#ifdef CO_REFERENCED_DEBUG
            os << std::endl;
            p->printHolders( os );
#endif
            os << enableFlush;
        }
        else
            os << "RP[ NULL ]";

        return os;
    }

    template< class T > inline std::string className( const RefPtr<T>& rp )
        { return className( rp.get( )); }
}

}
#endif //COBASE_REFPTR_H
