
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "staticSlaveCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "object.h"

#include <eq/base/scopedMutex.h>

using namespace eqNet;
using namespace eqBase;
using namespace std;

StaticSlaveCM::StaticSlaveCM( Object* object )
        : _object( object )
{
}

StaticSlaveCM::~StaticSlaveCM()
{
}

void StaticSlaveCM::applyInitialData( const void* data, const uint64_t size,
                                     const uint32_t version )
{
    EQASSERT( version == Object::VERSION_NONE );
    _object->applyInstanceData( data, size );
}
