
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "debug.h"

#include <sstream>
#include <string>
#include <time.h>

namespace
{
/**
 * Leave only last directory + file name, the beginning of the line
 * is replaceced with "....".
 */
std::string _getShortName( const char* name )
{
    std::string s( name );

    size_t pos = s.rfind( '/' );
    if( pos == std::string::npos || pos < 4 )
        return s;

    pos = s.rfind( '/', pos-1 );
    if( pos == std::string::npos || pos < 4 )
        return s;

    pos -= 4;
    s.replace( pos, 4, "...." );

    s = s.substr( pos );
    return s;
}
}// namespace

std::string formatDedugOut( const char* name, const char* f, const int l, const char* msg )
{
    std::stringstream ss;
    ss << _getShortName( name ) << "::" << f << "::" << l << msg;

    return ss.str();
}


static time_t _rawtime;

char * getTime()
{
    time( &_rawtime );
    return ctime( &_rawtime );
}
