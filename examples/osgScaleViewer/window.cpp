
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#include "window.h"

#include "config.h"
#include "node.h"
#include "pipe.h"

namespace osgScaleViewer
{
bool Window::configInitGL( const uint32_t initID )
{
    if( !eq::Window::configInitGL( initID ))
        return false;

    Window* sharedWindow = static_cast< Window* >( getSharedContextWindow( ));
    if( sharedWindow == this || sharedWindow == 0 ) // init GL stuff
    {
        Node* node = static_cast< Node* >( getNode( ));

        // The code below is not thread-safe, since various STL containers are
        // used within the OSG classes. Since this is init-only, a simple global
        // lock is acceptable.
        static eq::base::Lock lock;
        eq::base::ScopedMutex<> mutex( lock );
        
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
        _sceneView = sharedWindow->_sceneView;

    return true;
}

bool Window::configExitGL()
{
    _sceneView = 0;
    return eq::Window::configExitGL();
}

}
