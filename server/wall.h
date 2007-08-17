
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_WALL_H
#define EQS_WALL_H

#include <eq/base/base.h>
#include <eq/vmmlib/vmmlib.h>

#include <iostream>

namespace eqs
{
    /**
     * A wall defining a view frustum.
     * 
     * The three points describe the bottom left, bottom right and top left
     * coordinate of the wall in real-world coordinates.
     */
    class Wall
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

        bool operator == ( const Wall& rhs ) const;
        bool operator != ( const Wall& rhs ) const;

        vmml::Vector3f bottomLeft;
        vmml::Vector3f bottomRight;
        vmml::Vector3f topLeft;
    };

    std::ostream& operator << ( std::ostream& os, const Wall& wall );
}

#endif // EQS_WALL_H

