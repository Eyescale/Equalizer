
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

#ifndef CHANNEL_H
#define CHANNEL_H

#ifdef WIN32
#define EQ_IGNORE_GLEW
#endif

#include <eq/eq.h>

#include <osg/LightSource>
#include <osg/Matrix>
#include <osg/ref_ptr>
#include <osg/Node>
#include <osg/Image>

class FrameData;
class Pipe;

/**
 * The Channel renders the frames in frameDraw().
 * This is done by querying the pipe for the viewer and then asking the viewer
 * to render the scene.
 */
class Channel : public eq::Channel
{
public:
    Channel( eq::Window* parent );

protected:
    /** @sa eq::Channel::configInit() */
    virtual bool configInit( const uint32_t initID );

    /** @sa eq::Channel::configExit() */
    virtual bool configExit();

    /** @sa eq::Channel::frameClear() */
    virtual void frameClear( const uint32_t frameID );

    /** @sa eq::Channel::frameStart() */
    virtual void frameStart( const uint32_t frameID,
                             const uint32_t frameNumber );

    /** @sa eq::Channel::frameDraw() **/
    virtual void frameDraw( const uint32_t frameID );

    /** @sa eq::Channel::frameViewFinish() */
    void frameViewFinish( const uint32_t frameID );

    /** 
     * Gets the camera position and viewing direction information out of the
     * frame data and sets the view matrix of the camera of the viewer of
     * the pipe based on this information.
     * @param pipe the pipe.
     * @return the view matrix.
     */
    osg::Matrix setPlainViewMatrix( const Pipe* pipe ) const;

    /** 
     * Init the scene graph.
     * @return the group node.
     */
    osg::ref_ptr<osg::Group> initSceneGraph();

    /** 
     * Creates the light source.
     * @return the light source node.
     */
    osg::ref_ptr<osg::LightSource> createLightSource();

    /** 
     * Creates the default scenegraph (a red quad)
     * @return the root node.
     */
    osg::ref_ptr<osg::Node> createSceneGraph();

    /** 
     * Creates the scenegraph with an image displayed on a quad.
     * @return the root node.
     */
    osg::ref_ptr<osg::Node> createSceneGraph( osg::ref_ptr<osg::Image> image );

    /** 
     * Updates the scene graph with the new data of the frame data.
     * Currently this only sets the rotation of the quad.
     */
    void updateSceneGraph();

private:
    const osg::ref_ptr<osg::Node> _getModel();
    const osg::ref_ptr<osg::Image> _getImage();

    osg::ref_ptr<osg::Node> _model;
    osg::ref_ptr<osg::Image> _image;
};

#endif
