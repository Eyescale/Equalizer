
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

        void setFrameDataID( const uint32_t id )  
            { _clearInstanceData(); _frameDataID = id; }

        uint32_t           getFrameDataID()  const { return _frameDataID;  }
        eq::WindowSystem   getWindowSystem() const { return _windowSystem; }
        const std::string& getDataFilename() const { return _dataFilename; }
 
    protected:
        virtual const void* getInstanceData( uint64_t* size );
        virtual void applyInstanceData( const void* data, const uint64_t size );

        void setWindowSystem( const eq::WindowSystem windowSystem )
            { _clearInstanceData(); _windowSystem = windowSystem; }
  
        void setDataFilename( const std::string& dataFilename )
            { _clearInstanceData(); _dataFilename = dataFilename; }
  
    private:
        uint32_t         _frameDataID;
        eq::WindowSystem _windowSystem;
        std::string      _dataFilename; //!< volume raw data file name

        char* _instanceData;
        void  _clearInstanceData();
    };
}


#endif // EQ_VOL_INITDATA_H

