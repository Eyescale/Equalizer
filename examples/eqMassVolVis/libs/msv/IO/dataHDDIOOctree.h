
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__DATA_HDD_IO_OCTREE_H
#define MASS_VOL__DATA_HDD_IO_OCTREE_H

#include "dataHDDIO.h"

namespace massVolVis
{

/**
 * Octree uncompressed data loader
 */
class DataHDDIOOctree : public DataHDDIO
{
public:

    explicit DataHDDIOOctree( const VolumeFileInfo& fileInfo );

    virtual ~DataHDDIOOctree(){}

    void setBaseFileName( std::string& baseName ) { _baseName = baseName; }

    virtual bool read(  const uint32_t id,       void* dst );
    virtual bool write( const uint32_t id, const void* src );

    virtual bool allocateAllFiles( const uint32_t maxId );

    virtual constVolumeTreeBaseSPtr getTree() const;

private:
    /** Offset is 32 bit since we have rather small files (2GB), otherwice it has to be
        changed to 64! */
    void _getFileNameAndOffset( const uint32_t id, std::string& name, uint32_t& offset );

    std::string _baseName;  // base name of files withing the folder
};

}

#endif // MASS_VOL__DATA_HDD_IO_OCTREE_H




