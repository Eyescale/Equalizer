
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__DATA_HDD_IO_RAW_H
#define MASS_VOL__DATA_HDD_IO_RAW_H

#include "dataHDDIO.h"

namespace massVolVis
{

/**
 * One file raw data, with arbitrary number of bits per value
 */
class DataHDDIORaw: public DataHDDIO
{
public:
    explicit DataHDDIORaw( const VolumeFileInfo& fileInfo );

    virtual ~DataHDDIORaw(){}

    virtual void setDataFileName( const std::string& name );

    virtual bool read( const Box_i32& dim, void* dst );

private:
    std::vector<char> _tmp;
    int64_t           _oldZ;
    int64_t           _oldD;
};

}

#endif // MASS_VOL__DATA_HDD_IO_RAW_H






