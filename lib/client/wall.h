
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WALL_H
#define EQ_WALL_H

#include <eq/base/base.h>
#include <vmmlib/vmmlib.h>  // member
#include "viewport.h"
#include <iostream>

namespace eq
{
    class Projection;

    /**
     * A wall defining a view frustum.
     * 
     * The three points describe the bottom left, bottom right and top left
     * coordinate of the wall in real-world coordinates.
     */
    class EQ_EXPORT Wall
    {
    public:
        Wall();

        /** 
         * Resize the wall horizontally.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         */
        void resizeHorizontal( const float ratio );

        /** 
         * Resize the wall vertical.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         */
        void resizeVertical( const float ratio );
        
        void apply( const Viewport& viewport);

        //TODO /** Set the wall parameters from a projection description. */
        //Wall& operator = ( const Projection& projection );

        bool operator == ( const Wall& rhs ) const;
        bool operator != ( const Wall& rhs ) const;

        vmml::Vector3f bottomLeft;
        vmml::Vector3f bottomRight;
        vmml::Vector3f topLeft;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Wall& wall );
}
#endif // EQ_WALL_H

