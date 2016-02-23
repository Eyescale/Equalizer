
/*
 * Copyright (c) 2008-2016, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *                          Stefan Eilemann <eile@eyescale.ch>
 *                          Sarah Amsellem <sarah.amsellem@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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


void Channel::frameClear( const eq::uint128_t& frameID )
{
    glEnable( GL_SCISSOR_TEST );
    eq::Channel::frameClear( frameID );
}

void Channel::frameDraw( const eq::uint128_t& frameID )
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
        frustum.nearPlane(), frustum.farPlane( ));

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

void Channel::frameViewFinish( const eq::uint128_t& frameID )
{
    const Pipe *pipe = static_cast< const Pipe* >( getPipe( ));
    const FrameData& frameData = pipe->getFrameData();
    if( frameData.useStatistics( ))
    {
        applyBuffer();
        applyViewport();
        drawStatistics();
    }

    eq::Channel::frameViewFinish( frameID );
}

}
