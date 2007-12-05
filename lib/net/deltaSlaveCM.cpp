
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "deltaSlaveCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "object.h"
#include "objectDataIStream.h"

#include <eq/base/scopedMutex.h>

using namespace eqNet;
using namespace eqBase;
using namespace std;

DeltaSlaveCM::DeltaSlaveCM( Object* object )
        : FullSlaveCM( object )
{
}

DeltaSlaveCM::~DeltaSlaveCM()
{
}

void DeltaSlaveCM::_unpackOneVersion( ObjectDataIStream* is )
{
    EQASSERT( is );

    EQASSERTINFO( _version == is->getVersion() - 1, "Expected version " 
                  << _version << ", got " << is->getVersion() - 1 );
    
    _object->unpack( *is );
    _version = is->getVersion();
    EQLOG( LOG_OBJECTS ) << "unpacked v" << _version << ", id "
                         << _object->getID() << "." << _object->getInstanceID()
                         << endl;

    if( is->getRemainingBufferSize() > 0 || is->nRemainingBuffers() > 0 )
        EQWARN << "Object " << typeid( *_object ).name() 
            << " did not unpack all data" << endl;
}

