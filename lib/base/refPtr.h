
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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
    struct RefPtr_scast{};

    /**
     * A smart reference pointer
     */
    template<class T> class RefPtr 
    {
    public:
        RefPtr()                     : _ptr( 0 )         {}
        RefPtr( T* const ptr )       : _ptr( ptr )       { ref(); }
        RefPtr( const RefPtr& from ) : _ptr( from._ptr ) { ref(); }
        
        template<class from>
        RefPtr( RefPtr<from> const &f, RefPtr_scast )
            { _ptr = static_cast<T*>(f._ptr); ref(); }


        ~RefPtr() { unref(); _ptr = 0; }

        void ref()   { if(_ptr) _ptr->ref(); }
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

        bool operator == ( const RefPtr& rhs ) const 
            { return ( _ptr==rhs._ptr ); }
        bool operator != ( const RefPtr& rhs ) const
            { return ( _ptr!=rhs._ptr ); }
        bool operator <  ( const RefPtr& rhs ) const
            { return ( _ptr < rhs._ptr ); }
        bool operator >  ( const RefPtr& rhs ) const
            { return ( _ptr > rhs._ptr ); }
        bool operator ! () const               { return ( _ptr==0 ); }

        bool operator == ( const T* ptr ) const { return ( _ptr==ptr ); }
        bool operator != ( const T* ptr ) const { return ( _ptr!=ptr ); }
        bool operator <  ( const T* ptr ) const { return ( _ptr < ptr ); }
        bool operator >  ( const T* ptr ) const { return ( _ptr > ptr ); }

        T*       operator->()       
            { EQASSERTINFO( _ptr, typeid(*this).name( )); return _ptr; }
        const T* operator->() const
            { EQASSERTINFO( _ptr, typeid(*this).name( )); return _ptr; }
        T&       operator*()        
            { EQASSERTINFO( _ptr, typeid(*this).name( )); return *_ptr; }
        const T& operator*() const  
            { EQASSERTINFO( _ptr, typeid(*this).name( )); return *_ptr; }

        T*       get()                { return _ptr; }
        const T* get() const          { return _ptr; }
        T&       getReference()       { EQASSERT( _ptr ); return *_ptr; }
        const T& getReference() const { EQASSERT( _ptr ); return *_ptr; }

        bool isValid() const { return ( _ptr != 0 ); }
        
    private:
        T* _ptr;

        template<class U> friend class RefPtr;
    };

    // cast functions
    template<class from, class to> RefPtr<to> RefPtr_static_cast( RefPtr<from>
                                                                  const &f )
    { return RefPtr<to>( f, RefPtr_scast() ); }

    template< class T >
    inline std::ostream& operator << ( std::ostream& os, const RefPtr<T>& rp )
    {
        os << "RefPtr<" << rp.get() << ">";
        return os;
    }
}

}
#endif //EQBASE_REFPTR_H
