
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_RANGE_H
#define EQ_RANGE_H

#include <eq/base/base.h>

#include <iostream>

namespace eq
{
    /**
     * Holds a fractional range along with some methods for manipulation.
     */
    class Range 
    {
    public:
        /**
         * @name Constructors
         */
        //@{
        Range() : start(0.f), end(1.f)  {}

        Range( const float start_, const float end_ )
                : start(start_), end(end_) {}
        //@}

        void apply( const Range& rhs )
            {
                const float w = end-start;
                end    = start + rhs.end * w;
                start += rhs.start * w;
            }
        
        bool operator == ( const Range& rhs ) const
        {
            return start==rhs.start && end==rhs.end;
        }
        
        bool operator != ( const Range& rhs ) const
        {
            return start!=rhs.start || end!=rhs.end;
        }
        
        void invalidate() { start=0.f; end=0.f; }

        bool isValid() const 
            { return ( start>=0.f && end <=1.f && (end - start) >= 0.f ); }

        bool hasData() const { return  (end - start) > 0.f; }

        float start;
        float end;

        EQ_EXPORT static const Range ALL;
    };

    inline std::ostream& operator << ( std::ostream& os, const Range& range )
    {
        os << "range    [ " << range.start << " " << range.end << " ]";
        return os;
    }
}

#endif // EQ_RANGE_H
