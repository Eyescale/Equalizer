
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "user.h"

#include <eq/base/log.h>

#include <alloca.h>

using namespace eqNet;
using namespace std;

User::User(const uint id )
        : _id(id)
{
    INFO << "New user" << this << endl;
}

