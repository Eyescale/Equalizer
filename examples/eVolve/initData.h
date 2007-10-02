
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VOL_INITDATA_H
#define EQ_VOL_INITDATA_H

#include "eVolve.h"
#include "frameData.h"

#include <eq/eq.h>

namespace eVolve
{
    class InitData : public eqNet::Object
    {
    public:
        InitData();
        virtual ~InitData();

        void setFrameDataID( const uint32_t id )   { _frameDataID = id; }

        uint32_t           getFrameDataID() const  { return _frameDataID; }
        eq::WindowSystem   getWindowSystem() const { return _windowSystem;}
        bool               useCg() const           { return _useCg; }
        const std::string& getFilename()    const  { return _filename; }

    protected:
        virtual void getInstanceData( eqNet::DataOStream& os );
        virtual void applyInstanceData( eqNet::DataIStream& is );

        void setWindowSystem( const eq::WindowSystem windowSystem )
            { _windowSystem = windowSystem; }
        void disableCg()     { _useCg = false; }
        void setFilename( const std::string& filename ) { _filename = filename;}

    private:
        uint32_t         _frameDataID;
        eq::WindowSystem _windowSystem;
        bool             _useCg;
        std::string      _filename;
    };
}


#endif // EQ_VOL_INITDATA_H

