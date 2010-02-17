
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

#include "sceneReader.h"

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>

SceneReader::SceneReader()
{
}

osg::ref_ptr<osg::Node> SceneReader::readModel( const std::string& filename )
{
    osg::ref_ptr<osg::Node> root = osgDB::readNodeFile( filename );
    if ( !root.valid( )) 
    {
        EQERROR << "Failed to load model." << std::endl;
        return root;
    }

    //Optimize scenegraph
    osgUtil::Optimizer optOSGFile;
    optOSGFile.optimize( root.get( ));
    return root;
}

osg::ref_ptr<osg::Image> SceneReader::readImage( const std::string& filename )
{
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile( filename );
    if( !image.valid( ))
        EQERROR << "Failed to load image." << std::endl;

    return image;
}
