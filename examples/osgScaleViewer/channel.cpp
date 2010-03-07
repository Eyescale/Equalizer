
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

#include "channel.h"

#include "config.h"
#include "pipe.h"
#include "quad.h"
#include "util.h"
#include "window.h"

#include <osg/MatrixTransform>
#include <osg/Texture2D>

namespace osgScaleViewer
{

Channel::Channel( eq::Window* parent )
    : eq::Channel( parent )
{
}


void Channel::frameClear( const uint32_t frameID )
{
    glEnable( GL_SCISSOR_TEST );
    eq::Channel::frameClear( frameID );
}

void Channel::frameDraw( const uint32_t frameID )
{
    // setup OpenGL State
    eq::Channel::frameDraw( frameID );

    // - 2D viewport
    Window *window = static_cast< Window* >( getWindow( ));
    osg::ref_ptr< SceneView > view = window->getSceneView();
    
    const eq::PixelViewport& pvp = getPixelViewport();
    view->setViewport( pvp.x, pvp.y, pvp.w, pvp.h );

    // - Stereo
    view->setDrawBufferValue( getDrawBuffer( ));
    const eq::ColorMask& colorMask = getDrawBufferMask();

    osgUtil::RenderStage* stage = view->getRenderStage();
    osg::ref_ptr< osg::ColorMask > osgMask = stage->getColorMask();
    osgMask->setMask( colorMask.red, colorMask.green, colorMask.blue, true );

    // - Frustum (Projection matrix)
    const eq::Frustumf& frustum = getFrustum();
    view->setProjectionMatrixAsFrustum( 
        frustum.left(), frustum.right(), frustum.bottom(), frustum.top(),
        frustum.near_plane(), frustum.far_plane( ));

    // - Camera (Model Matrix)
    const Pipe *pipe = static_cast< const Pipe* >( getPipe( ));
    const FrameData& frameData = pipe->getFrameData();
    
    const eq::Vector3f position = frameData.getCameraPosition();
    const eq::Vector3f lookAt = frameData.getCameraLookAtPoint();
    const eq::Vector3f upVector = frameData.getCameraUpVector();

    const osg::Vec3f pos( position.x(), position.y(), position.z( ));
    const osg::Vec3f look( lookAt.x(), lookAt.y(), lookAt.z( ));
    const osg::Vec3f up( upVector.x(), upVector.y(), upVector.z( ));
  
    view->setViewMatrixAsLookAt( pos, look, up );

    // - Frustum position (View Matrix)
    osg::Matrix headView = view->getViewMatrix();
    headView.postMult( vmmlToOsg( getHeadTransform( )));
    view->setViewMatrix( headView );

    // - Render
    view->cull();
    view->draw();
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

}
