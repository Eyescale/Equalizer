
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_RANGE_H
#define EQ_RANGE_H

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
        //*{
        Range() : start(0.f), end(1.f)  {}

        Range( const float start_, const float end_ )
                : start(start_), end(end_) {}
        //*}

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
            { return ( start>=0.f && end <=1.f && (end - start) > 0.f ); }

        float start;
        float end;

        static const Range ALL;
    };

    inline std::ostream& operator << ( std::ostream& os, const Range& range )
    {
        if( range.isValid( ))
            os << "range    [ " << range.start << " " << range.end << " ]";
        return os;
    }
}

#endif // EQ_RANGE_H
