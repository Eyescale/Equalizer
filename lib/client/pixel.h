
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_PIXEL_H
#define EQ_PIXEL_H

#include <eq/base/base.h>
#include <eq/base/log.h>

namespace eq
{
    class Pixel;
    std::ostream& operator << ( std::ostream& os, const Pixel& pixel );

    /**
     * Holds a pixel decomposition specification along with some methods for
     * manipulation.
     *
     * The w, h size determines how many contributors are sending pixels to the
     * destination. The x, y index determines the pixel of the contributor
     * within the decomposition pixel.
     */
    class Pixel 
    {
    public:
        /**
         * @name Constructors
         */
        //@{
        Pixel() : x( 0 ), y( 0 ), w( 1 ), h( 1 )  {}

        Pixel( const uint32_t x_, const uint32_t y_, 
               const uint32_t w_, const uint32_t h_ )
                : x( x_ ), y( y_ ), w( w_ ), h( h_ ) {}
        //@}

        void apply( const Pixel& rhs )
            {
                if( !isValid() || !rhs.isValid( ))
                    return;

                x = x * rhs.w + rhs.x;
                w *= rhs.w;
                y = y * rhs.h + rhs.y;
                h *= rhs.h;
            }

        bool operator == ( const Pixel& rhs ) const
        {
            return x==rhs.x && y==rhs.y && w==rhs.w && h==rhs.h;
        }
        
        bool operator != ( const Pixel& rhs ) const
        {
            return x!=rhs.x || y!=rhs.y || w!=rhs.w || h!=rhs.h;
        }
        
        void invalidate() 
            { x = y = w = h = 0; }

        void validate()
            {
                if( isValid( )) return;
                EQWARN << "Invalid " << *this << std::endl;
                if( w == 0 ) w = 1;
                if( h == 0 ) h = 1;
                if( x >= w ) x = 0;
                if( y >= h ) y = 0;
                EQWARN << "Corrected " << *this << std::endl;
            }

        bool isValid() const { return ( w>0 && x<w && h>0 && y<h ); }

        uint32_t x;
        uint32_t y;
        uint32_t w;
        uint32_t h;

        EQ_EXPORT static const Pixel ALL;
    };

    inline std::ostream& operator << ( std::ostream& os, const Pixel& pixel )
    {
        if( pixel.isValid( ))
            os << "pixel     [ " << pixel.x << ' ' << pixel.y
               << ' ' << pixel.w << ' ' << pixel.h << " ]";
        return os;
    }
}

#endif // EQ_PIXEL_H
