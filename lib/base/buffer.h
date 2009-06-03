
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
        Buffer() : data(0), size(0), _maxSize(0) {}

        /** Destruct the buffer. */
        ~Buffer() { clear(); }

        /** Flush the buffer, deleting all data. */
        void clear() { if( data ) free( data ); data=0; size=0; _maxSize=0; }

        /** Copy constructor, transfers ownership to new Buffer. */
        Buffer( Buffer& from )
            {
                data = from.data; size = from.size; _maxSize = from._maxSize;
                from.data = 0; from.size = 0; from._maxSize = 0;
            }

        /** Assignment operator, copies data from Buffer. */
        const Buffer& operator = ( Buffer& from )
            {
                replace( from.data, from.size );
                return *this;
            }

        /** Direct access to the element at the given index. */
        T&       operator[]( const size_t position )
            { EQASSERT( size > position ); return data[ position ]; }
        /** Direct const access to the element at the given index. */
        const T& operator[]( const size_t position) const
            { EQASSERT( size > position ); return data[ position ]; }

        /** 
         * Ensure that the buffer contains at least newSize elements, retaining
         * existing data.
         */
        void resize( const uint64_t newSize )
            { 
                size = newSize;
                if( newSize <= _maxSize )
                    return;
                
                const size_t nBytes = newSize * sizeof( T );
                if( data )
                    data = static_cast< T* >( realloc( data, nBytes ));
                else
                    data = static_cast< T* >( malloc( nBytes ));
                
                _maxSize = newSize;
            }

        /** 
         * Ensure that the buffer contains at least newSize elements,
         * potentially deleting existing data.
         */
        void reserve( const uint64_t newSize )
            { 
                if( newSize <= _maxSize )
                    return;

                if( data )
                    free( data );
                
                data = static_cast< T* >( malloc( newSize * sizeof( T )));
                _maxSize = size;
            }

        /** Append addSize elements to the buffer, increasing the size. */
        void append( const T* addData, const uint64_t addSize )
            {
                EQASSERT( addData );
                EQASSERT( addSize );

                const uint64_t oldSize = size;
                resize( oldSize + addSize );
                memcpy( data + oldSize, addData, addSize * sizeof( T ));
            }

        /** Append one element to the buffer, increasing the size. */
        void append( const T& element )
            {
                resize( size + 1 );
                data[ size - 1 ] = element;
            }

        /** Replace the existing data with new data. */
        void replace( const void* newData, const uint64_t newSize )
            {
                EQASSERT( newData );
                EQASSERT( newSize );

                reserve( newSize );
                memcpy( data, newData, newSize * sizeof( T ));
                size = newSize;
            }

        /** Swap the buffer contents with another Buffer. */
        void swap( Buffer& buffer )
            {
                T*             tmpData    = buffer.data;
                const uint64_t tmpSize    = buffer.size;
                const uint64_t tmpMaxSize = buffer._maxSize;

                buffer.data = data;
                buffer.size = size;
                buffer._maxSize = _maxSize;

                data     = tmpData;
                size     = tmpSize;
                _maxSize = tmpMaxSize;
            }

        /** @return the maximum size of the buffer. */
        uint64_t getMaxSize() const { return _maxSize; }

        /** A pointer to the data. */
        T*    data;

        /** The number of valid items in data. */
        uint64_t size;

    private:
        /** The allocation size of the buffer. */
        uint64_t _maxSize;
    };

    typedef Buffer< uint8_t > Bufferb;
}

}
#endif //EQBASE_BUFFER_H
