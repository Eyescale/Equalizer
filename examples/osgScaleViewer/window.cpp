
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
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

#include "window.h"

#include "config.h"
#include "node.h"
#include "pipe.h"

namespace osgScaleViewer
{
bool Window::configInitGL( const eq::uint128_t& initID )
{
    if( !eq::Window::configInitGL( initID ))
        return false;

    const eq::Window* sharedWindow = getSharedContextWindow();
    if( sharedWindow == this || sharedWindow == 0 ) // init GL stuff
    {
        Node* node = static_cast< Node* >( getNode( ));

        // The code below is not thread-safe, since various STL containers are
        // used within the OSG classes. Since this is init-only, a simple global
        // lock is acceptable.
        static lunchbox::Lock lock;
        lunchbox::ScopedMutex<> mutex( lock );

        _sceneView = new SceneView;
        _sceneView->setDefaults( SceneView::STANDARD_SETTINGS );
        _sceneView->setFrameStamp( node->getFrameStamp( ));
        _sceneView->init();
        _sceneView->getState()->setContextID( node->getUniqueContextID( ));
        _sceneView->getRenderStage()->setColorMask( new osg::ColorMask );

        osg::ref_ptr< osg::Node > model = node->getModel();
        _sceneView->setSceneData( model );
    }
    else
        _sceneView = static_cast< const Window* >( sharedWindow )->_sceneView;

    return true;
}

bool Window::configExitGL()
{
    _sceneView = 0;
    return eq::Window::configExitGL();
}

}
