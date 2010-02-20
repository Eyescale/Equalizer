
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

#ifndef NODE_H
#define NODE_H

#include <eq/eq.h>

#include <osg/Image>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Node>

namespace osgScaleViewer
{
    class Node : public eq::Node
    {
    public:
        /** 
         * Creates a Node.
         * @param parent the node's parent.
         */
        Node( eq::Config* parent );

        int32_t getUniqueContextID() { return ++_contextID; }
        osg::ref_ptr< osg::Node > getModel() { return _model; }
        osg::ref_ptr< osg::FrameStamp > getFrameStamp() { return _frameStamp; }

    protected:
        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();
        virtual void frameStart( const uint32_t frameID,
                                 const uint32_t frameNumber );

    private:
        eq::base::a_int32_t _contextID;
        osg::ref_ptr< osg::Node > _model;
        osg::ref_ptr< osg::FrameStamp > _frameStamp;
        osg::ref_ptr< osg::NodeVisitor > _updateVisitor;

        osg::ref_ptr< osg::Node > _createSceneGraph();
        osg::ref_ptr< osg::Node > _createSceneGraph( osg::ref_ptr<osg::Image> );
        osg::ref_ptr< osg::Group > _initSceneGraph();
        osg::ref_ptr< osg::LightSource > _createLightSource();
    };

}
#endif
