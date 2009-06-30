
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

#ifndef EQ_WALL_H
#define EQ_WALL_H

#include <eq/base/base.h>

#include <eq/client/types.h>
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
    class Wall
    {
    public:
        EQ_EXPORT Wall();

        /** 
         * Resize the wall horizontally.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         */
        EQ_EXPORT void resizeHorizontal( const float ratio );

        /** 
         * Resize the wall vertically.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         */
        EQ_EXPORT void resizeVertical( const float ratio );
        
        /** 
         * Resize the wall on the left side.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         */
        EQ_EXPORT void resizeLeft( const float ratio );

        /** 
         * Resize the wall on the right side.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         */
        EQ_EXPORT void resizeRight( const float ratio );

        /** 
         * Resize the wall on the top side.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         */
        EQ_EXPORT void resizeTop( const float ratio );

        /** 
         * Resize the wall on the bottom side.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         */
        EQ_EXPORT void resizeBottom( const float ratio );

        /** Compute the sub-frustum for a 2D area on the full wall. */
        EQ_EXPORT void apply( const Viewport& viewport);

        /** Set the wall parameters from a projection description. */
        EQ_EXPORT Wall& operator = ( const Projection& projection );

        /** @return the width of the wall. */
        float getWidth() const { return (bottomRight - bottomLeft).length(); }

        /** @return the height of the wall. */
        float getHeight() const { return (topLeft - bottomLeft).length(); }

        EQ_EXPORT bool operator == ( const Wall& rhs ) const;
        EQ_EXPORT bool operator != ( const Wall& rhs ) const;

        Vector3f bottomLeft;
        Vector3f bottomRight;
        Vector3f topLeft;
        
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

