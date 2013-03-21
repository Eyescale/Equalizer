
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "dataHDDIO.h"


namespace massVolVis
{

// not all children will implement these
bool DataHDDIO::read(  const Box_i32&,       void* ) { throw "unimplemented"; }
bool DataHDDIO::write( const Box_i32&, const void* ) { throw "unimplemented"; }
bool DataHDDIO::read(  const uint32_t,       void* ) { throw "unimplemented"; }
bool DataHDDIO::write( const uint32_t, const void* ) { throw "unimplemented"; }
bool DataHDDIO::allocateAllFiles( const uint32_t   ) { throw "unimplemented"; }

bool DataHDDIO::isBlockSizeValid() const
{
    return getBlockSize_() > 0;
}


bool DataHDDIO::isSourceSizeValid() const
{
    return getSourceDims().getAreaSize() > 0 && getBytesNum() > 0;
}

constVolumeTreeBaseSPtr DataHDDIO::getTree() const { throw "unimplemented"; }

}

