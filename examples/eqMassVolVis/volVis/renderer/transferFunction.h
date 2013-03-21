
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__TRANSFER_FUNCTION_H
#define MASS_VOL__TRANSFER_FUNCTION_H

#include <msv/types/types.h>

#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace massVolVis
{

struct TransferFunction
{
    TransferFunction();

    bool load( const std::string& fName );

    void initDefault();

    std::vector<byte> rgba;
    std::vector<byte> sda;
};

co::DataOStream& operator << ( co::DataOStream&, const TransferFunction& );
co::DataIStream& operator >> ( co::DataIStream&, TransferFunction& );


} //namespace massVolVis

#endif //MASS_VOL__TRANSFER_FUNCTION_H

