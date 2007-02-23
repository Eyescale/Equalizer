
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "staticMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "packets.h"

using namespace eqNet;
using namespace eqBase;
using namespace std;

StaticMasterCM::StaticMasterCM( Object* object )
        : _object( object )
{}

StaticMasterCM::~StaticMasterCM()
{}

const void* StaticMasterCM::getInitialData( uint64_t* size, uint32_t* version )
{
    *version = Object::VERSION_NONE;
    return _object->getInstanceData( size );
}
