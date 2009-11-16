
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
 *                   , Sarah Amsellem <sarah.amsellem@gmail.com>
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

#ifndef EQ_SUBPIXEL_H
#define EQ_SUBPIXEL_H

#include <eq/base/base.h>
#include <eq/base/log.h>

namespace eq
{
    class SubPixel;
    std::ostream& operator << ( std::ostream& os, const SubPixel& subPixel );

    /**
     * Holds a subpixel decomposition specification along with some methods for
     * manipulation.
     *
     * The index represents the contributor ID within the subpixel decomposition.
     * The size determines how many contributors are performing anti-aliasing.
     */
    class SubPixel
    {
    public:
        /**
         * @name Constructors
         */
        //@{
        SubPixel() : index( 0 ), size( 1 )  {}

        SubPixel( const uint32_t index_, const uint32_t size_ )
                : index( index_ ), size( size_ ) {}
        //@}

        void apply( const SubPixel& rhs )
        {
            if( !isValid() || !rhs.isValid( ))
                return;

            index = index * rhs.size + rhs.index;
            size  *= rhs.size;
        }

        bool operator == ( const SubPixel& rhs ) const
        {
            return index==rhs.index && size==rhs.size;
        }

        bool operator != ( const SubPixel& rhs ) const
        {
            return index != rhs.index || size != rhs.size;
        }

        void invalidate() { index = size = 0; }

        void validate()
        {
            if( isValid( )) return;
            EQWARN << "Invalid " << *this << std::endl;
            if( index >= size ) index = 0;
            if( size == 0 )     size = 1;
            EQWARN << "Corrected " << *this << std::endl;
        }

        bool isValid() const { return ( index < size ); }

        uint32_t index;
        uint32_t size;

        EQ_EXPORT static const SubPixel ALL;
    };

    inline std::ostream& operator << ( std::ostream& os, const SubPixel& subPixel )
    {
        if( subPixel.isValid( ))
            os << "subpixel  [ " << subPixel.index << ' ' << subPixel.size << " ]";
        return os;
    }
}

#endif // EQ_SUBPIXEL_H
