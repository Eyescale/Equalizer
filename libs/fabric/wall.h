
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQFABRIC_WALL_H
#define EQFABRIC_WALL_H

#include <eq/fabric/api.h>

#include <eq/types.h>
#include <iostream>

namespace eq
{
namespace fabric
{
    class Projection;

    /**
     * A wall defining a view frustum.
     * 
     * The three points describe the bottom left, bottom right and top left
     * coordinate of the wall in real-world coordinates (meters).
     */
    class Wall
    {
    public:
        /** Construct a new wall description. */
    	EQFABRIC_API Wall();

        /** Construct a new wall description with default values. */
    	EQFABRIC_API Wall( const Vector3f& bottomLeft, const Vector3f& bottomRight,
                        const Vector3f& topLeft );

        /** 
         * Resize the wall horizontally.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         * @version 1.0
         */
    	EQFABRIC_API void resizeHorizontal( const float ratio );

        /** 
         * Resize the wall vertically.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         * @version 1.0
         */
    	EQFABRIC_API void resizeVertical( const float ratio );
        
        /** 
         * Resize the wall on the left side.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         * @version 1.0
         */
    	EQFABRIC_API void resizeLeft( const float ratio );

        /** 
         * Resize the wall on the right side.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         * @version 1.0
         */
    	EQFABRIC_API void resizeRight( const float ratio );

        /** 
         * Resize the wall on the top side.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         * @version 1.0
         */
    	EQFABRIC_API void resizeTop( const float ratio );

        /** 
         * Resize the wall on the bottom side.
         * 
         * @param ratio the amount by which the wall is grown or shrunk.
         * @version 1.0
         */
    	EQFABRIC_API void resizeBottom( const float ratio );

        /**
         * Compute the sub-frustum for a 2D area on the full wall.
         * @version 1.0
         */
    	EQFABRIC_API void apply( const Viewport& viewport);

        /**
         * Set the wall parameters from a projection description.
         * @version 1.0
         */
    	EQFABRIC_API Wall& operator = ( const Projection& projection );

        /** @return the width of the wall. @version 1.0 */
        float getWidth() const { return (bottomRight - bottomLeft).length(); }

        /** @return the height of the wall. @version 1.0 */
        float getHeight() const { return (topLeft - bottomLeft).length(); }

        /** @return true if the two walls are identical. @version 1.0 */
        EQFABRIC_API bool operator == ( const Wall& rhs ) const;

        /** @return true if the two walls are not identical. @version 1.0 */
        EQFABRIC_API bool operator != ( const Wall& rhs ) const;

        Vector3f bottomLeft;  //!< The bottom-left corner
        Vector3f bottomRight; //!< The bottom-right corner
        Vector3f topLeft;     //!< The top-left corner
        
        /** The reference system type of the wall. */
        enum Type
        {
            TYPE_FIXED, //!< A fixed mounted projection wall
            TYPE_HMD    //!< A wall fixed to the observer (head-mounted display)
        };
        Type type; //!< The wall type
    };

    EQFABRIC_API std::ostream& operator << ( std::ostream&, const Wall& );
    EQFABRIC_API std::ostream& operator << ( std::ostream&, const Wall::Type& );
}
}
#endif // EQFABRIC_WALL_H

