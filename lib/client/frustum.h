
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FRUSTUM_H
#define EQ_FRUSTUM_H

#include <eq/vmmlib/VMMLib.h>
#include <iostream>

namespace eq
{
    /** 
     * Frustum helper class.
     */
    struct Frustum
    {
        Frustum() : 
                left(-1.0f), right(1.0f),
                top(-1.0f),  bottom(1.0f),
                near(0.1f),  far(100.0f) {}
        
        vmml::Matrix4f computeMatrix() const;
        void           adjustNear( const float near );

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
