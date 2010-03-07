
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

#ifndef OSG_SV_WINDOW_H
#define OSG_SV_WINDOW_H

#include <eq/eq.h>
#include "sceneView.h"

namespace osgScaleViewer
{
    /**
     * A window represents an OpenGL drawable and context
     */
    class Window : public eq::Window
    {
    public:
        Window( eq::Pipe* parent ) : eq::Window( parent ) {}
        
        osg::ref_ptr< SceneView > getSceneView() { return _sceneView; }

    protected:
        virtual ~Window() {}
        virtual bool configInitGL( const uint32_t initID );
        virtual bool configExitGL();

    private:
        osg::ref_ptr< SceneView > _sceneView;
    };
}

#endif // OSG_SV_WINDOW_H
