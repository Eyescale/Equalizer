
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "global.h"
#include "nodeFactory.h"

using namespace eq;
using namespace std;

NodeFactory* Global::_nodeFactory = createNodeFactory();
string       Global::_server;

std::ostream& eq::operator << ( std::ostream& os, const IAttrValue value )
{
    if( value > ON ) // ugh
        os << static_cast<int>( value );
    else
        os << ( value == UNDEFINED ? "undefined" :
                value == OFF       ? "off" :
                value == ON        ? "on" : 
                value == AUTO      ? "auto" :
                value == NICEST    ? "nicest" : "ERROR"  );
    return os;
}
