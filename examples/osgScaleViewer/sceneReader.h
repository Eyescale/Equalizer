
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef OSGSV_SCENEREADER_H
#define OSGSV_SCENEREADER_H

#include <eq/eq.h>

#include <osg/ref_ptr>
#include <osg/Node>
#include <osg/Image>

namespace osgScaleViewer
{
    class SceneReader
    {
    public:
        /** 
         * Constructs a new SceneReader.
         */
        SceneReader();

        /** 
         * Reads an osg model from its filename.
         * @param filename the model filename.
         * @return the root node of the scenegraph.
         */
        osg::ref_ptr<osg::Node> readModel( const std::string& filename );

        /** 
         * Reads an osg image from its filename.
         * @param filename the image filename.
         * @return the image object.
         */
        osg::ref_ptr<osg::Image> readImage( const std::string& filename );
    };
}
#endif
