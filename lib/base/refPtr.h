
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_REFPTR_H
#define EQBASE_REFPTR_H

#include <eq/base/base.h>
#include <eq/base/debug.h>

#include <iostream>
#include <typeinfo>
#include <stdlib.h>

namespace eq
{
namespace base
{
    /**
     * A smart reference pointer.
     *
     * Relies on the held object to implement ref and unref correctly.
     */
    template<class T> class RefPtr 
    {
    public:
        /** Construct a new, empty reference pointer. @version 1.0 */
        RefPtr()                     : _ptr( 0 )         {}
        /** Construct a reference pointer from a C pointer. @version 1.0 */
        RefPtr( T* const ptr )       : _ptr( ptr )       { ref(); }
        /** Construct a copy of a reference pointer. @version 1.0 */
        RefPtr( const RefPtr& from ) : _ptr( from._ptr ) { ref(); }
        /** Destruct this reference pointer. @version 1.0 */
        ~RefPtr() { unref(); _ptr = 0; }

        /** Artificially reference the held object. @version 1.0 */
        void ref()   { if(_ptr) _ptr->ref(); }

        /** Artificially dereference the held object. @version 1.0 */
        void unref() 
        {
            if(_ptr)
            {
#ifndef NDEBUG
                const bool abondon = (_ptr->getRefCount() == 1);
                _ptr->unref();
                if( abondon ) 
                    _ptr = 0;
#else
                _ptr->unref();
#endif
            }
        }

        /** Assign another RefPtr to this reference pointer. @version 1.0 */
        RefPtr& operator = ( const RefPtr& rhs )
            {
                if( _ptr == rhs._ptr )
                    return *this;

                T* tmp = _ptr;
                _ptr = rhs._ptr;
                ref();
                if( tmp ) tmp->unref();
                return *this;
            }

        /** Assign a C pointer to this reference pointer. @version 1.0 */
        RefPtr& operator = ( T* ptr )
            {
                if( _ptr == ptr )
                    return *this;

                T* tmp = _ptr;
                _ptr = ptr;
                ref();
                if( tmp ) tmp->unref();
                return *this;
            }

        /**
         * @return true if both reference pointers hold the same C pointer.
         * @version 1.0
         */
        bool operator == ( const RefPtr& rhs ) const 
            { return ( _ptr==rhs._ptr ); }

        /**
         * @return true if both reference pointer hold different C pointer.
         * @version 1.0
         */
        bool operator != ( const RefPtr& rhs ) const
            { return ( _ptr!=rhs._ptr ); }

        /**
         * @return true if the left RefPtr is smaller then the right.
         * @version 1.0
         */
        bool operator <  ( const RefPtr& rhs ) const
            { return ( _ptr < rhs._ptr ); }

        /**
         * @return true if the right RefPtr is smaller then the left.
         * @version 1.0
         */
        bool operator >  ( const RefPtr& rhs ) const
            { return ( _ptr > rhs._ptr ); }

        /** @return true if the RefPtr is empty. @version 1.0 */
        bool operator ! () const               { return ( _ptr==0 ); }

        /**
         * @return true if the reference pointers holds the C pointer.
         * @version 1.0
         */
        bool operator == ( const T* ptr ) const { return ( _ptr==ptr ); }

        /**
         * @return true if the reference pointers does not hold the C pointer
         * @version 1.0
         */
        bool operator != ( const T* ptr ) const { return ( _ptr!=ptr ); }

        /** Access the held object. @version 1.0 */
        T*       operator->()       
            { EQASSERTINFO( _ptr, typeid(*this).name( )); return _ptr; }
        /** Access the held object. @version 1.0 */
        const T* operator->() const
            { EQASSERTINFO( _ptr, typeid(*this).name( )); return _ptr; }
        /** Access the held object. @version 1.0 */
        T&       operator*()        
            { EQASSERTINFO( _ptr, typeid(*this).name( )); return *_ptr; }
        /** Access the held object. @version 1.0 */
        const T& operator*() const  
            { EQASSERTINFO( _ptr, typeid(*this).name( )); return *_ptr; }

        /** @return the C pointer. @version 1.0 */
        T*       get()                { return _ptr; }
        /** @return the C pointer. @version 1.0 */
        const T* get() const          { return _ptr; }

        /** @return true if the RefPtr holds a non-0 pointer. @version 1.0 */
        bool isValid() const { return ( _ptr != 0 ); }
        
    private:
        T* _ptr;
    };

    /** Print the reference pointer to the given output stream. */
    template< class T >
    inline std::ostream& operator << ( std::ostream& os, const RefPtr<T>& rp )
    {
        os << "RefPtr<" << rp.get() << ">";
        return os;
    }
}

}
#endif //EQBASE_REFPTR_H
