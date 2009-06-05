
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_PLY_INITDATA_H
#define EQ_PLY_INITDATA_H

#include "eqPly.h"
#include "frameData.h"

#include <eq/eq.h>

namespace eqPly
{
    class InitData : public eq::net::Object
    {
    public:
        InitData();
        virtual ~InitData();

        void setFrameDataID( const uint32_t id )   { _frameDataID = id; }

        uint32_t           getFrameDataID() const   { return _frameDataID; }
        eq::WindowSystem   getWindowSystem() const  { return _windowSystem; }
        mesh::RenderMode   getRenderMode() const    { return _renderMode; }
        bool               useGLSL() const          { return _useGLSL; }
        bool               useInvertedFaces() const { return _invFaces; }

    protected:
        virtual void getInstanceData( eq::net::DataOStream& os );
        virtual void applyInstanceData( eq::net::DataIStream& is );

        void setWindowSystem( const eq::WindowSystem windowSystem )
            { _windowSystem = windowSystem; }
        void setRenderMode( const mesh::RenderMode renderMode )
            { _renderMode = renderMode; }
        void enableGLSL()          { _useGLSL  = true; }
        void enableInvertedFaces() { _invFaces = true; }

    private:
        uint32_t         _frameDataID;
        eq::WindowSystem _windowSystem;
        mesh::RenderMode _renderMode;
        bool             _useGLSL;
        bool             _invFaces;
    };
}


#endif // EQ_PLY_INITDATA_H

