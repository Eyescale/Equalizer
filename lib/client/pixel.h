
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIXEL_H
#define EQ_PIXEL_H

#include <eq/base/log.h>

namespace eq
{
    class Pixel;
    std::ostream& operator << ( std::ostream& os, const Pixel& pixel );

    /**
     * Holds a pixel decomposition specification along with some methods for
     * manipulation.
     *
     * The size determines how many contributors are sending pixels to the
     * destination. The index determines the pixel of the contributor within the
     * decomposition pixel.
     */
    class Pixel 
    {
    public:
        /**
         * @name Constructors
         */
        //*{
        Pixel() : index( 0 ), size( 1 )  {}

        Pixel( const uint32_t index_, const uint32_t size_ )
                : index( index_ ), size( size_ ) {}
        //*}

        void apply( const Pixel& rhs )
            {
                if( !isValid() || !rhs.isValid( ))
                    return;

                index = index * rhs.size + rhs.index;
                size *= rhs.size;
            }

        bool operator == ( const Pixel& rhs ) const
        {
            return index==rhs.index && size==rhs.size;
        }
        
        bool operator != ( const Pixel& rhs ) const
        {
            return index!=rhs.index || size!=rhs.size;
        }
        
        void invalidate() 
            { index = 0; size = 0; }

        void validate()
            {
                if( isValid( )) return;
                EQWARN << "Invalid " << *this << std::endl;
                if( size == 0 )
                    size = 1;
                if( index >= size )
                    index = 0;
                EQWARN << "Corrected " << *this << std::endl;
            }

        bool isValid() const { return ( size > 0 && index < size ); }

        uint32_t index;
        uint32_t size;

        static const Pixel ALL;
    };

    inline std::ostream& operator << ( std::ostream& os, const Pixel& pixel )
    {
        if( pixel.isValid( ))
            os << "pixel     [ " << pixel.index << " " << pixel.size << " ]";
        return os;
    }
}

#endif // EQ_PIXEL_H
