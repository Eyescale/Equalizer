
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef UTIL_STR_H
#define UTIL_STR_H

#include <sstream>
#include <fstream>

namespace strUtil
{

std::string trim( std::string const& source, char const* delims = " \t\r\n" );


std::string toLower( const std::string& str );

void toLower( std::string* str );


template <class T>
inline std::string toString( const T& t )
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

}

#endif //UTIL_STR_H
