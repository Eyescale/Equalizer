
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_INITDATA_H
#define EQ_PLY_INITDATA_H

#include "eqPly.h"
#include "frameData.h"

#include <eq/eq.h>

class InitData : public eqNet::Object
{
public:
    InitData();
    InitData( const void* data, const uint64_t size );
    virtual ~InitData();

    virtual uint32_t getTypeID() const { return TYPE_INITDATA; }

    void setFrameData( eqBase::RefPtr<FrameData> frameData );

    eqBase::RefPtr<FrameData> getFrameData();
    void releaseFrameData();

    const std::string& getFilename() const { return _filename; }

protected:
    const void* getInstanceData( uint64_t* size );

    void setFilename( const std::string& filename );


private:
    uint32_t _frameDataID;
    eqBase::PerThread< FrameData* > _frameData;

    std::string _filename;

    char* _instanceData;
    void  _clearInstanceData();
};



#endif // EQ_PLY_INITDATA_H

