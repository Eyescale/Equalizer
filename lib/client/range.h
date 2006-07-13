
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_RANGE_H
#define EQ_RANGE_H

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
        Range() : start(0), end(1)  {}

        Range( const float start, const float end )
                : start(start), end(end) {}
        //*}

        Range& operator *= ( const Range& rhs )
            {
                const float w = end-start;
                end    = start + rhs.end * w;
                start += rhs.start * w;
                return *this;
            }

        void invalidate() { start=0; end=0; }

        bool isValid() const 
            { return ( start>=0 && end <=1 && (end - start) > 0 ); }
        bool isFull() const { return ( start==0 && end==1 ); }

        float start;
        float end;
    };

    inline std::ostream& operator << ( std::ostream& os, const Range& range )
    {
        os << "range    [ " << range.start << " " << range.end << " ]";
        return os;
    }
}

#endif // EQ_RANGE_H
