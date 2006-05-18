
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FRUSTUM_H
#define EQ_FRUSTUM_H

#include <iostream>

namespace eq
{
    /** 
     * Frustum helper class.
     */
    struct Frustum
    {
        Frustum() : 
                left(-1.), right(1.),
                top(-1.),  bottom(1.),
                near(.1),  far(100.) {}
        
        void computeMatrix( float matrix[16] ) const;

        float left;
        float right;
        float top;
        float bottom;
        float near;
        float far;
    };

    inline std::ostream& operator << ( std::ostream& os, const Frustum& frustum)
    {
        os << "frustum [" << frustum.left << "," << frustum.right << " " 
           << frustum.top << "," << frustum.bottom << " " 
           << frustum.near << "," << frustum.far << "]";
        
        return os;
    }
}

#endif // EQ_FRUSTUM_H
