
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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
        void setModelID( const uint32_t id )       { _modelID = id; }

        uint32_t           getFrameDataID() const   { return _frameDataID; }
        uint32_t           getModelID() const       { return _modelID; }
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
        uint32_t         _modelID;
        eq::WindowSystem _windowSystem;
        mesh::RenderMode _renderMode;
        bool             _useGLSL;
        bool             _invFaces;
    };
}


#endif // EQ_PLY_INITDATA_H

