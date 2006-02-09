
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WALL_H
#define EQ_WALL_H


namespace eq
{
    // Definitions for common display systems in meters
#   define WALL_20INCH_16x10 {                            \
        { -.21672, -.13545, -1, },                        \
        {  .21672, -.13545, -1, },                        \
        { -.21672,  .13545, -1, }}

#   define WALL_12INCH_4x3 {                              \
        { -.12294, -.09220, -1, },                        \
        {  .12294, -.09220, -1, },                        \
        { -.12294,  .09220, -1, }}

    /**
     * A wall definition defining a view frustum.
     * 
     * The three points describe the bottom left, bottom right and top left
     * coordinate of the wall in real-world coordinates.
     */
    struct Wall
    {
        float bottomLeft[3];
        float bottomRight[3];
        float topLeft[3];
    };
}

#endif // EQ_WALL_H

