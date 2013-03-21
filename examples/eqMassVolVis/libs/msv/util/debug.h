
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef UTIL_DEBUG_H
#define UTIL_DEBUG_H

#include <iostream>
#include <iomanip>

//#define NDEBUG

char * getTime();

std::string formatDedugOut( const char* name, const char* f, const int l, const char* msg );

#define LOG_ERROR std::cerr << std::setw(50) << std::left << formatDedugOut(__FILE__,__FUNCTION__,__LINE__,"  Error: ").c_str()

//#define LOG_INFO  std::cout << std::setw(50) << std::left << formatDedugOut(__FILE__,__FUNCTION__,__LINE__,"         ").c_str()
//#define NL        std::endl << std::setw(50) << " "
#define LOG_INFO_ std::cout << " >>>>   " << getTime()
#define LOG_INFO  std::cout
#define NL        std::endl

#define FAIL(x) do{ LOG_ERROR << x << std::endl; throw "Critical error"; } while(0)

#endif //UTIL_DEBUG_H
