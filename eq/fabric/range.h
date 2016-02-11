
/* Copyright (c) 2006-2015, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQFABRIC_RANGE_H
#define EQFABRIC_RANGE_H

#include <eq/fabric/api.h>
#include <lunchbox/bitOperation.h>
#include <algorithm> // min,max
#include <iostream>

namespace eq
{
namespace fabric
{
/** A fractional database range with methods for manipulation. */
class Range
{
public:
    /** @name Constructors */
    //@{
    /** Construct a new range covering all data. @version 1.0 */
    Range() : start(0.f), end(1.f)  {}

    /** Construct a new range with default values. @version 1.0 */
    Range( const float start_, const float end_ )
        : start(start_), end(end_) {}
    //@}

    /** @name Data Access */
    //@{
    /** @return true if the two ranges are identical. @version 1.0 */
    bool operator == ( const Range& rhs ) const
        { return start==rhs.start && end==rhs.end; }

    /** @return true if the two ranges are not identical. @version 1.0 */
    bool operator != ( const Range& rhs ) const
        { return start!=rhs.start || end!=rhs.end; }

    /** @return the interval spanned by this range. @version 1.3 */
    float getSize() const { return end - start; }

    /** @internal Invalidate the database range. */
    void invalidate() { start=0.f; end=0.f; }

    /** @internal @return true if the database range is valid. */
    bool isValid() const
        { return ( start>=0.f && end <=1.f && (end - start) >= 0.f ); }

    /** @internal @return true if the database range covers some data. */
    bool hasData() const { return  (end - start) > 0.f; }

    /** @internal Apply (accumulate) another database range. */
    void apply( const Range& rhs )
    {
        const float w = end-start;
        end    = start + rhs.end * w;
        start += rhs.start * w;
    }

    /** Merge the two ranges (form the union). @version 1.11 */
    void merge( const Range& rhs )
    {
        start = std::min( start, rhs.start );
        end = std::max( end, rhs.end );
    }

    float start; //!< The start position
    float end;   //!< The end position

    EQFABRIC_API static const Range ALL; //!< A full database range
};

inline std::ostream& operator << ( std::ostream& os, const Range& range )
{
    os << "range    [ " << range.start << " " << range.end << " ]";
    return os;
}
}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::Range& value )
{
    byteswap( value.start );
    byteswap( value.end );
}
}
#endif // EQFABRIC_RANGE_H
