
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__RAM_DATA_ELEMENT_H
#define MASS_VOL__RAM_DATA_ELEMENT_H

#include <msv/types/types.h>
#include <msv/tree/nodeId.h>
#include "timeStamp.h"

namespace massVolVis
{

/**
 *  Contains data for one brick
 */
class RAMDataElement
{
public:
    explicit RAMDataElement( const uint32_t dataSize )
        : _nodeId( 0 )
        , _lastAccess( 0 )
        , _dataSize( dataSize )
        , _data( 0 )
        , _readLocked( 0 )
    {
    }

    ~RAMDataElement()
    {
        if( _data )
        {
            delete [] _data;
            _data = 0;
        }
    }

    void allocate()
    {
        if( _data == 0 && _dataSize > 0 )
            _data = new byte[ _dataSize ];
    }

    inline       uint32_t size() const { return _dataSize; }
    inline       byte*    data()       { return _data;     }
    inline const byte*    data() const { return _data;     }

    inline TimeStamp lastAccess() const { return _lastAccess; }

    inline void setLastAccess( TimeStamp l ) { if(  isWriteLocked() || l == TimeStamp::max() ) throw "can't setLastAccess"; _lastAccess = l; }
    inline void writeUnlock(   TimeStamp l ) { if( !isWriteLocked() || l == TimeStamp::max() ) throw "can't writeUnlock";   _lastAccess = l; }

    inline void writeLock() { if( isWriteLocked() || isReadLocked() ) throw "can't lockWriteAccess"; _lastAccess = TimeStamp::max(); }

    inline bool isWriteLocked() { return _lastAccess == TimeStamp::max(); }
    inline bool isWriteFree()   { return _lastAccess != TimeStamp::max(); }


    inline void reset() { _lastAccess.reset(); _nodeId = 0; _readLocked = 0; }

    inline uint64_t getNodeId() const { return _nodeId; }

    inline void setNodeId( const NodeId id ) { if( isReadLocked() ) throw "can't setNodeId"; _nodeId = id; }

    inline void readLock()   { if( _readLocked > 10 || isWriteLocked()) throw "can't readLock";   _readLocked++; } // can be only locked once per GPU
    inline void readUnlock() { if( _readLocked == 0 || isWriteLocked()) throw "can't readUnlock"; _readLocked--; } // can be only unlocked once per lock
    inline bool isReadFree()   const { return _readLocked == 0; }
    inline bool isReadLocked() const { return _readLocked != 0; }
private:
    NodeId    _nodeId;
    TimeStamp _lastAccess;
    uint32_t  _dataSize;
    byte*     _data;
    byte      _readLocked; // data is unlocked if 0
};


}

#endif //MASS_VOL__RAM_DATA_ELEMENT_H
