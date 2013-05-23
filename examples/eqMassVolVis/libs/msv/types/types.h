
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__TYPES_H
#define MASS_VOL__TYPES_H

#include <stdint.h>
#include <vector>

//#define CHAR_BIT 8 // have no clue why this has to be defined manually =(

typedef uint8_t byte;

typedef std::vector<uint32_t> vec32_t;

// Allows to print type string, like this: std::cout << TypeParseTraits<var>::name << std::endl;
// Usable names should be defined before in types.cpp
// Usefull in case RTTI (e.g. "typeid(var).name()" ) doesn't work or gives unusable results.
template<typename T> struct TypeParseTraits { static const char* name; };

// used to register TypeParseTraits names
#define REGISTER_PARSE_TYPE(X) template <> struct TypeParseTraits<X> \
    { static const char* name; } ; const char* TypeParseTraits<X>::name = #X


#endif // MASS_VOL__TYPES_H

