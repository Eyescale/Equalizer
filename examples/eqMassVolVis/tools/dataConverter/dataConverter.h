
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__DATA_CONVERTER_H
#define MASS_VOL__DATA_CONVERTER_H

#include <msv/IO/dataHDDIO.h>

#include <string>

namespace massVolVis
{

/**
 * Interface to data loading from HDD.
 */
class DataConverter
{
public:
    enum Data
    {
        NONE = 0,
        RAW,
        RAW_DER,
        OCT,
        OCT_HEAD
    };

    DataConverter() : _dataIn( 0 ), _dataOut( 0 ), _inType( NONE ), _outType( NONE ) {}

    void setDataReader( VolumeFileInfo* dataIn );
    void setDataWriter( VolumeFileInfo* dataOut );

    void setReaderType( Data type ) { _inType  = type;  }
    void setWriterType( Data type ) { _outType = type; }

    void convert();

    static const std::string& getHelp();

private:

    VolumeFileInfo* _dataIn;
    VolumeFileInfo* _dataOut;
    DataHDDIOSPtr   _hddIn;
    DataHDDIOSPtr   _hddOut;

    Data _inType;
    Data _outType;
};

}

#endif // MASS_VOL__DATA_CONVERTER_H






