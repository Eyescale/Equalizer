
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
            { _clearInstanceData(); _data.frameDataID = id; }

        uint32_t           getFrameDataID() const  { return _data.frameDataID; }
        eq::WindowSystem   getWindowSystem() const { return _data.windowSystem;}
        bool               useVBOs() const         { return _data.useVBOs; }
        bool               useShaders() const      { return _data.useShaders; }
        const std::string& getFilename()    const  { return _filename; }

    protected:
        virtual const void* getInstanceData( uint64_t* size );
        virtual void applyInstanceData( const void* data, const uint64_t size );

        void setWindowSystem( const eq::WindowSystem windowSystem )
            { _clearInstanceData(); _data.windowSystem = windowSystem; }
        void enableVBOs() { _clearInstanceData(); _data.useVBOs = true; }
        void enableShaders() { _clearInstanceData(); _data.useShaders = true; }
        void setFilename( const std::string& filename )
            { _clearInstanceData(); _filename = filename; }

    private:
        void  _clearInstanceData();
        char* _instanceData;

        struct StaticData
        {
            StaticData()
                    : frameDataID( EQ_UNDEFINED_UINT32 )
                    , windowSystem( eq::WINDOW_SYSTEM_NONE )
                    , useVBOs( false ), useShaders( false )
                {}
            uint32_t         frameDataID;
            eq::WindowSystem windowSystem;
            bool             useVBOs;
            bool             useShaders;
        };

        StaticData  _data;
        std::string _filename;

    };
}


#endif // EQ_PLY_INITDATA_H

