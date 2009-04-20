
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifndef EQ_WALL_H
#define EQ_WALL_H

#include <eq/base/base.h>

#include <vmmlib/vmmlib.h>  // member
#include <iostream>

namespace eq
{
    class Projection;
    class Viewport;

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
         * Resize the wall vertically.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         */
        void resizeVertical( const float ratio );
        
        /** 
         * Resize the wall on the left side.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         */
        void resizeLeft( const float ratio );

        /** 
         * Resize the wall on the right side.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         */
        void resizeRight( const float ratio );

        /** 
         * Resize the wall on the top side.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         */
        void resizeTop( const float ratio );

        /** 
         * Resize the wall on the bottom side.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         */
        void resizeBottom( const float ratio );

        void apply( const Viewport& viewport);

        /** Set the wall parameters from a projection description. */
        Wall& operator = ( const Projection& projection );

        bool operator == ( const Wall& rhs ) const;
        bool operator != ( const Wall& rhs ) const;

        vmml::Vector3f bottomLeft;
        vmml::Vector3f bottomRight;
        vmml::Vector3f topLeft;
        
        enum Type
        {
            TYPE_FIXED,
            TYPE_HMD
        };
        Type type;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Wall& wall );
}
#endif // EQ_WALL_H

