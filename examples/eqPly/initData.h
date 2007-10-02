
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_INITDATA_H
#define EQ_PLY_INITDATA_H

#include "eqPly.h"
#include "frameData.h"

#include <eq/eq.h>

namespace eqPly
{
    class InitData : public eqNet::Object
    {
    public:
        InitData();
        virtual ~InitData();

        void setFrameDataID( const uint32_t id )   { _frameDataID = id; }

        uint32_t           getFrameDataID() const  { return _frameDataID; }
        eq::WindowSystem   getWindowSystem() const { return _windowSystem;}
        bool               useVBOs() const         { return _useVBOs; }
        bool               useShaders() const      { return _useShaders; }
        const std::string& getFilename()    const  { return _filename; }

    protected:
        virtual void getInstanceData( eqNet::DataOStream& os );
        virtual void applyInstanceData( eqNet::DataIStream& is );

        void setWindowSystem( const eq::WindowSystem windowSystem )
            { _windowSystem = windowSystem; }
        void enableVBOs()    { _useVBOs = true; }
        void enableShaders() { _useShaders = true; }
        void setFilename( const std::string& filename ) { _filename = filename;}

    private:
        uint32_t         _frameDataID;
        eq::WindowSystem _windowSystem;
        bool             _useVBOs;
        bool             _useShaders;
        std::string      _filename;
    };
}


#endif // EQ_PLY_INITDATA_H

