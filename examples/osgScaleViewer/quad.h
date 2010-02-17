
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@equalizergraphics.com>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#ifndef QUAD_H
#define QUAD_H

#include <osg/ref_ptr>
#include <osg/Node>

class Quad
{
public:
    /**
     * Creates and returns the root node of the quad.
     * @return the root node holding the quad.
     */
    osg::ref_ptr<osg::Node> createQuad() const;

    /**
     * Creates and returns the root node of the textured quad.
     * @param width the width of the image.
     * @param height the height of the image.
     */
    osg::ref_ptr<osg::Node> createQuad( int width, int height ) const;

private:
    /** 
     * Helper function for createQuad().
     */
    osg::ref_ptr<osg::Drawable> createDrawable() const;
    
    /** 
     * Helper function for createQuad( width, height ).
     */
    osg::ref_ptr<osg::Drawable> createDrawable( int width, int height ) const;

};

#endif // QUAD_H
