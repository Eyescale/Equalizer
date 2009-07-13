
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_BUFFER_H
#define EQBASE_BUFFER_H

#include <eq/base/nonCopyable.h> // base class
#include <eq/base/debug.h>       // EQASSERT macro

namespace eq
{
namespace base
{
    /**
     * A simple memory buffer with some helper functions.
     *
     * For bigger data (>100k) using a std::vector< uint8_t > has a high
     * overhead when resizing (>1ms).
     * This buffer just memcpy's elements, i.e., it should be used on PODs only
     * since the copy constructor or assignment operator is not called on the
     * copied elements. Primarily used for binary data, e.g., in eq::Image.
     */
    template< typename T >
    class Buffer
    {
    public:
        /** Construct a new, empty buffer. */
        Buffer() : _data(0), _size(0), _maxSize(0) {}

        /** Destruct the buffer. */
        ~Buffer() { clear(); }

        /** Flush the buffer, deleting all _data. */
        void clear() 
            { if( _data ) free( _data ); _data=0; _size=0; _maxSize=0; }

        /** Copy constructor, transfers ownership to new Buffer. */
        Buffer( Buffer& from )
            {
                _data = from._data; _size = from._size; _maxSize =from._maxSize;
                from._data = 0; from._size = 0; from._maxSize = 0;
            }

        /** Assignment operator, copies _data from Buffer. */
        const Buffer& operator = ( Buffer& from )
            {
                replace( from._data, from._size );
                return *this;
            }

        /** Direct access to the element at the given index. */
        T&       operator[]( const size_t position )
            { EQASSERT( _size > position ); return _data[ position ]; }
        /** Direct const access to the element at the given index. */
        const T& operator[]( const size_t position) const
            { EQASSERT( _size > position ); return _data[ position ]; }

        /** 
         * Ensure that the buffer contains at least newSize elements, retaining
         * existing _data.
         */
        void resize( const uint64_t newSize )
            { 
                _size = newSize;
                if( newSize <= _maxSize )
                    return;
                
                const size_t nBytes = newSize * sizeof( T );
                if( _data )
                    _data = static_cast< T* >( realloc( _data, nBytes ));
                else
                    _data = static_cast< T* >( malloc( nBytes ));
                
                _maxSize = newSize;
            }

        /** 
         * Ensure that the buffer contains at least newSize elements,
         * potentially deleting existing _data.
         */
        void reserve( const uint64_t newSize )
            { 
                if( newSize <= _maxSize )
                    return;

                if( _data )
                    free( _data );
                
                _data = static_cast< T* >( malloc( newSize * sizeof( T )));
                _maxSize = _size;
            }

        /** Append addSize elements to the buffer, increasing the _size. */
        void append( const T* addData, const uint64_t addSize )
            {
                EQASSERT( addData );
                EQASSERT( addSize );

                const uint64_t oldSize = _size;
                resize( oldSize + addSize );
                memcpy( _data + oldSize, addData, addSize * sizeof( T ));
            }

        /** Append one element to the buffer, increasing the _size. */
        void append( const T& element )
            {
                resize( _size + 1 );
                _data[ _size - 1 ] = element;
            }

        /** Replace the existing _data with new _data. */
        void replace( const void* newData, const uint64_t newSize )
            {
                EQASSERT( newData );
                EQASSERT( newSize );

                reserve( newSize );
                memcpy( _data, newData, newSize * sizeof( T ));
                _size = newSize;
            }

        /** Swap the buffer contents with another Buffer. */
        void swap( Buffer& buffer )
            {
                T*             tmpData    = buffer._data;
                const uint64_t tmpSize    = buffer._size;
                const uint64_t tmpMaxSize = buffer._maxSize;

                buffer._data = _data;
                buffer._size = _size;
                buffer._maxSize = _maxSize;

                _data     = tmpData;
                _size     = tmpSize;
                _maxSize = tmpMaxSize;
            }

        /** @return a pointer to the _data. */
        T* getData() { return _data; }

        /** @return a const pointer to the _data. */
        const T* getData() const { return _data; }

        /**
         * Set the size of the buffer without changing its allocation.
         *
         * This method only modifies the size parameter. If the current
         * allocation of the buffer is too small, it asserts, returns false and
         * does not change the size.
         */
        bool setSize( const uint64_t size )
            {
                EQASSERT( size <= _maxSize );
                if( size > _maxSize )
                    return false;

                _size = size;
                return true;
            }
                    
        /** @return the size. */
        uint64_t getSize() const { return _size; }
        
        /** @return true if the buffer is empty, false if not. */
        bool isEmpty() const { return (_size==0); }
        
        /** @return the maximum _size of the buffer. */
        uint64_t getMaxSize() const { return _maxSize; }

    private:
        /** A pointer to the _data. */
        T*    _data;

        /** The number of valid items in _data. */
        uint64_t _size;

        /** The allocation _size of the buffer. */
        uint64_t _maxSize;
    };

    typedef Buffer< uint8_t > Bufferb;
}

}
#endif //EQBASE_BUFFER_H
