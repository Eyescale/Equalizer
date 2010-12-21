
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *               2009, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#ifndef EQFABRIC_SUBPIXEL_H
#define EQFABRIC_SUBPIXEL_H

#include <eq/fabric/api.h>
#include <co/base/log.h>

namespace eq
{
namespace fabric
{
    class SubPixel;
    std::ostream& operator << ( std::ostream& os, const SubPixel& subPixel );

    /**
     * Holds a subpixel decomposition specification along with some methods for
     * manipulation.
     *
     * The index represents the contributor ID within the subpixel
     * decomposition.  The size determines how many contributors are performing
     * anti-aliasing or any other subpixel decomposition.
     */
    class SubPixel
    {
    public:
        /** @name Constructors */
        //@{
        /** Construct an empty subpixel specification. @version 1.0 */
        SubPixel() : index( 0 ), size( 1 )  {}

        /**
         * Construct a subpixel specification with default values.
         * @version 1.0
         */
        SubPixel( const uint32_t index_, const uint32_t size_ )
                : index( index_ ), size( size_ ) {}
        //@}

        /** Apply (accumulate) another subpixel specification. @internal */
        void apply( const SubPixel& rhs )
        {
            if( !isValid() || !rhs.isValid( ))
                return;

            index = index * rhs.size + rhs.index;
            size  *= rhs.size;
        }

        /**
         * @return true if the two subpixel specifications are identical.
         * @version 1.0
         */
        bool operator == ( const SubPixel& rhs ) const
        {
            return index==rhs.index && size==rhs.size;
        }

        /**
         * @return true if the two subpixel specifications are not identical.
         * @version 1.0
         */
        bool operator != ( const SubPixel& rhs ) const
        {
            return index != rhs.index || size != rhs.size;
        }

        /** Make the subpixel specification invalid. @internal */
        void invalidate() { index = size = 0; }

        /** Make the subpixel specification valid. @internal */
        void validate()
        {
            if( isValid( )) return;
            EQWARN << "Invalid " << *this << std::endl;
            if( index >= size ) index = 0;
            if( size == 0 )     size = 1;
            EQWARN << "Corrected " << *this << std::endl;
        }

        /** @return true if the pixel specification is valid. @internal */
        bool isValid() const { return ( index < size ); }

        uint32_t index; //!< The contributor id
        uint32_t size;  //!< Total number of contributors

        EQFABRIC_API static const SubPixel ALL;
    };

    inline std::ostream& operator << ( std::ostream& os,
                                       const SubPixel& subPixel )
    {
        if( subPixel.isValid( ))
            os << "subpixel  [ " << subPixel.index << ' ' << subPixel.size
               << " ]";
        return os;
    }
}
}

#endif // EQFABRIC_SUBPIXEL_H
