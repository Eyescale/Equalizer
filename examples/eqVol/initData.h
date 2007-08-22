
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VOL_INITDATA_H
#define EQ_VOL_INITDATA_H

#include "eqVol.h"
#include "frameData.h"

#include <eq/eq.h>

namespace eqVol
{
    class InitData : public eqNet::Object
    {
    public:
        InitData();
        virtual ~InitData();

        void setFrameDataID( const uint32_t id )  
            { _clearInstanceData(); _frameDataID = id; }

        uint32_t           getFrameDataID()  const { return _frameDataID;  }
        const std::string& getDataFilename() const { return _dataFilename; }
        const std::string& getInfoFilename() const { return _infoFilename; }
 
    protected:
        virtual const void* getInstanceData( uint64_t* size );
        virtual void applyInstanceData( const void* data, const uint64_t size );

        void setDataFilename( const std::string& dataFilename )
            { _clearInstanceData(); _dataFilename = dataFilename; }

        void setInfoFilename( const std::string& infoFilename )
            { _clearInstanceData(); _infoFilename = infoFilename; }

    private:
        uint32_t    _frameDataID;
        std::string _dataFilename;	//!< volume raw data file name
        std::string _infoFilename;	//!< volume info file name

        char* _instanceData;
        void  _clearInstanceData();
    };
}


#endif // EQ_VOL_INITDATA_H

