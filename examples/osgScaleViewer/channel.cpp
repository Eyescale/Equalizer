
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

#include "channel.h"

#include "osgEqViewer.h"
#include "pipe.h"
#include "util.h"
#include "config.h"
#include "quad.h"

#include <osg/MatrixTransform>
#include <osg/Texture2D>

Channel::Channel( eq::Window* parent )
    : eq::Channel( parent )
    , _model( 0 )
    , _image( 0 )
{
}

bool Channel::configInit( const uint32_t initID )
{
    if( !eq::Channel::configInit( initID ))
        return false;

    Pipe *pipe = static_cast<Pipe*>( getPipe( ));
    osg::ref_ptr< OSGEqViewer > viewer = pipe->getViewer();
    
    // display the scenegraph if a valid model has been loaded
    osg::ref_ptr<osg::Node> model = _getModel();

    // Setting the scene data is not thread-safe, since various std::vectors in
    // the SG get resized. Since this is init-only, a simple lock is ok.
    static eq::base::Lock lock;
    eq::base::ScopedMutex<> mutex( lock );

    if( model.valid( ))
    {
        viewer->setSceneData( model.get( ));
        return true;
    }

    // display the image on a quad if a valid image has been loaded
    osg::ref_ptr<osg::Image> image = _getImage();
    if( image.valid( ))
    {
        viewer->setSceneData( correctCoordSys( 
            createSceneGraph( image.get( )).get( )));    
        return true;
    }

    // set the scene to render
    viewer->setSceneData( correctCoordSys( createSceneGraph().get( )));
    std::cout << "set scene data draw quad" << std::endl;

    return true;
}

bool Channel::configExit()
{
    _model = 0;
    _image = 0;
    return eq::Channel::configExit();
}

osg::Matrix Channel::setPlainViewMatrix( const Pipe* pipe ) const
{
    const FrameData& frameData = pipe->getFrameData();
    
    const eq::Vector3f position = frameData.getCameraPosition();
    const eq::Vector3f lookAt = frameData.getCameraLookAtPoint();
    const eq::Vector3f upVector = frameData.getCameraUpVector();

    const osg::Vec3f pos( position.x(), position.y(), position.z( ));
    const osg::Vec3f view( lookAt.x(), lookAt.y(), lookAt.z( ));
    const osg::Vec3f up( upVector.x(), upVector.y(), upVector.z( ));
  
    osg::ref_ptr< OSGEqViewer > viewer = pipe->getViewer();
    viewer->getCamera()->setViewMatrixAsLookAt( pos, view, up );

    return viewer->getCamera()->getViewMatrix();
}

void Channel::frameStart( const uint32_t frameID,
                          const uint32_t frameNumber )
{
    updateSceneGraph();
    eq::Channel::frameStart( frameID, frameNumber );
}

void Channel::frameDraw( const uint32_t frameID )
{
    // setup OpenGL State
    eq::Channel::frameDraw( frameID );

    Pipe *pipe = static_cast<Pipe*>( getPipe( ));
    osg::ref_ptr< OSGEqViewer > viewer = pipe->getViewer();
    
    viewer->setViewport( this );

    // set a view matrix and make sure it is multiplied with the head
    // matrix of Equalizer
    osg::Matrix headView = setPlainViewMatrix( pipe );
    headView.postMult( vmmlToOsg( getHeadTransform( )));
    viewer->getCamera()->setViewMatrix( headView );
    viewer->getCamera()->setLODScale( .001f );

    viewer->frame();
}

void Channel::frameViewFinish( const uint32_t frameID )
{
    const Pipe *pipe = static_cast< const Pipe* >( getPipe( ));
    const FrameData& frameData = pipe->getFrameData();
    if( !frameData.useStatistics( ))
        return;

    applyBuffer();
    applyViewport();
    drawStatistics();
}

osg::ref_ptr<osg::Group> Channel::initSceneGraph()
{
    osg::ref_ptr<osg::Group> root = new osg::Group();
    root->setDataVariance( osg::Object::STATIC );

    // enable lighting and LIGHT0 in root node
    osg::StateSet* state = root->getOrCreateStateSet();
    state->setMode( GL_LIGHTING, osg::StateAttribute::ON );
    state->setMode( GL_LIGHT0, osg::StateAttribute::ON );

    // lightsource
    osg::ref_ptr<osg::LightSource> ls0 = createLightSource();

    // translation of lightsource (with light0)
    osg::Matrix matrix;
    osg::ref_ptr<osg::MatrixTransform> lightTranslateNode = 
             new osg::MatrixTransform();

    matrix.makeTranslate( 0.f, -5.f, 0.f );
    lightTranslateNode->setMatrix( matrix );
    lightTranslateNode->addChild( ls0.get( ));
    lightTranslateNode->setDataVariance( osg::Object::STATIC );

    root->addChild( lightTranslateNode.get( ));

    return root.get();
}

osg::ref_ptr<osg::LightSource> Channel::createLightSource()
{
    osg::ref_ptr<osg::Light> light0 = new osg::Light();
    light0->setLightNum( 0 ); //light0 is now GL-Light0
    light0->setPosition( osg::Vec4( 0.f, -50.f, 0.f, 1.f ));
    light0->setAmbient( osg::Vec4( 1.0f, 1.0f, 1.0f, 0.0f ));
    light0->setDiffuse( osg::Vec4( 1.0f, 1.0f, 1.0f, 0.0f ));
    light0->setDirection( osg::Vec3( 0.f, 1.f, 0.f ));
    light0->setSpotCutoff( 30.f );
    light0->setSpotExponent( 50.0f );

    osg::ref_ptr<osg::LightSource> ls0 = new osg::LightSource();
    ls0->setLight( light0.get( ));
    ls0->setDataVariance( osg::Object::STATIC );

    return ls0.get();
}

osg::ref_ptr<osg::Node> Channel::createSceneGraph()
{
    // init scene graph
    osg::ref_ptr<osg::Group> root = initSceneGraph(); 

    // draw a red quad
    Quad quad;
    osg::ref_ptr<osg::Node> geometryChild = quad.createQuad();
    root->addChild( geometryChild );

    return root.get();
}

osg::ref_ptr<osg::Node> Channel::createSceneGraph(
                                 osg::ref_ptr<osg::Image> image )
{
    // init scene graph
    osg::ref_ptr<osg::Group> root = initSceneGraph(); 

    // det the image as a texture
    osg::Texture2D* texture = new osg::Texture2D();
    texture->setImage( image );
    
    // draw a textured quad
    Quad quad;
    osg::ref_ptr<osg::Node> geometryChild = 
            quad.createQuad( image->s(), image->t( ));

    osg::StateSet* stateOne = new osg::StateSet();
    stateOne->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );
    geometryChild->setStateSet( stateOne );

    root->addChild( geometryChild );

    return root.get();
}

void Channel::updateSceneGraph()
{
    // empty for now
}

osg::ref_ptr<osg::Node> Channel::correctCoordSys(
            osg::ref_ptr<osg::Node> nodeToRotate )
{
    return nodeToRotate;
    osg::Matrix matrixX;
    osg::ref_ptr<osg::MatrixTransform> staticRotationNodeX =
             new osg::MatrixTransform();

    // -90 degree rotation around the X axis
    matrixX.makeRotate( -osg::PI_2, osg::Vec3( 1., 0., 0. ));
    staticRotationNodeX->setMatrix( matrixX );

    staticRotationNodeX->addChild( nodeToRotate.get( ));
    staticRotationNodeX->setDataVariance( osg::Object::STATIC );

    return staticRotationNodeX.get();
}

const osg::ref_ptr<osg::Node> Channel::_getModel()
{
    Config* config = static_cast<Config*>( getConfig( ));

    if( !_model )
        _model = config->getModel();

    return _model;
}

const osg::ref_ptr<osg::Image> Channel::_getImage()
{
    Config* config = static_cast<Config*>( getConfig( ));

    if( !_image )
        _image = config->getImage();

    return _image;
}
