
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "global.h"
#include "nodeFactory.h"

using namespace eq;
using namespace std;

EQ_EXPORT eq::NodeFactory* eq::Global::_nodeFactory = 0;
string eq::Global::_server;

EQ_EXPORT std::ostream& eq::operator << ( std::ostream& os, const IAttrValue value )
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
