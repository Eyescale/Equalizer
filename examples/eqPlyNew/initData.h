
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

        void setFrameDataID( const uint32_t id )  
            { _clearInstanceData(); _frameDataID = id; }

        uint32_t           getFrameDataID() const  { return _frameDataID; }
        eq::WindowSystem   getWindowSystem() const { return _windowSystem; }
        const std::string& getFilename()    const  { return _filename; }

    protected:
        virtual const void* getInstanceData( uint64_t* size );
        virtual void applyInstanceData( const void* data, const uint64_t size );

        void setWindowSystem( const eq::WindowSystem windowSystem )
            { _clearInstanceData(); _windowSystem = windowSystem; }
        void setFilename( const std::string& filename )
            { _clearInstanceData(); _filename = filename; }

    private:
        uint32_t         _frameDataID;
        eq::WindowSystem _windowSystem;
        std::string      _filename;

        char* _instanceData;
        void  _clearInstanceData();
    };
}


#endif // EQ_PLY_INITDATA_H

