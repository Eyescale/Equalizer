
#include "str.h"

#include <algorithm>


namespace strUtil
{

std::string trim( std::string const& source, char const* delims )
{
    std::string result( source );
    std::string::size_type index = result.find_last_not_of( delims );

    if( index != std::string::npos )
        result.erase( ++index );

    index = result.find_first_not_of( delims );
    if( index != std::string::npos )
        result.erase( 0, index );
    else
        result.erase();

    return result;
}


std::string toLower( const std::string& str )
{
    std::string result = str;

    std::transform( result.begin(), result.end(), result.begin(), (int(*)(int))std::tolower );

    return result;
}


void toLower( std::string* str )
{
    std::transform( str->begin(), str->end(), str->begin(), (int(*)(int))std::tolower );
}


}
