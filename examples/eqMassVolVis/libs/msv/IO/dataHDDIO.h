
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */

#ifndef MASS_VOL__DATA_HDD_IO_H
#define MASS_VOL__DATA_HDD_IO_H

#include <msv/types/box.h>
#include <msv/types/types.h>
#include <msv/IO/volumeFileInfo.h>

#include <msv/types/nonCopyable.h>

#include <string>

#include <boost/shared_ptr.hpp>


namespace massVolVis
{

class VolumeTreeBase;
typedef boost::shared_ptr<       VolumeTreeBase >      VolumeTreeBaseSPtr;
typedef boost::shared_ptr< const VolumeTreeBase > constVolumeTreeBaseSPtr;

/**
 * Interface to data loading from HDD.
 */
class DataHDDIO : protected VolumeFileInfo, private NonCopyable
{
private:
    DataHDDIO(){ throw "shouldn't call"; }

public:
    DataHDDIO( const VolumeFileInfo& fileInfo ) : VolumeFileInfo( fileInfo ) {}
    virtual ~DataHDDIO(){}

    // tree is correct only if the object was constructed from fileInfo and
    // dimensions were not changed!
    virtual constVolumeTreeBaseSPtr getTree() const;

    /**
     * Closes open files, etc.
     */
    virtual void cleanup() {};

    /**
     * Allocates all files for writing.
     */
    virtual bool allocateAllFiles( const uint32_t maxId );

    /**
     * Reads dim data from HDD. User has to make sure that there is enought space
     * allocated in dst, depending on the format.
     */
    virtual bool read(  const Box_i32& dim,       void* dst );
    /**
     * Writes dim data to HDD.
     */
    virtual bool write( const Box_i32& dim, const void* src );

    /**
     * Reads dim data from HDD based on data id.
     */
    virtual bool read(  const uint32_t id,       void* dst );
    /**
     * Writes dim data to HDD.
     */
    virtual bool write( const uint32_t id, const void* src );

    virtual bool isBlockSizeValid()  const;
    virtual bool isSourceSizeValid() const;

    virtual uint32_t getMaxBlockSize()                   const { return VolumeFileInfo::getBlockSize_(); }
    virtual uint32_t getBlockSize_()                     const { return VolumeFileInfo::getBlockSize_(); }
    virtual uint32_t getBlockSize_( const uint32_t pos ) const { return VolumeFileInfo::getBlockSize_(); }
    virtual     byte getBytesNum()                       const { return VolumeFileInfo::getBytesNum();   }

    using VolumeFileInfo::getBlockDim;
    using VolumeFileInfo::getBorderDim;
    using VolumeFileInfo::getBytesNum;
    using VolumeFileInfo::getTFFileName;
    using VolumeFileInfo::getBlockAndBordersDim;

    bool isTfFileNameDefined() const { return isAttributeSet( VolumeFileInfo::TF_FILE ); }
};

}

#endif // MASS_VOL__DATA_HDD_IO_H






