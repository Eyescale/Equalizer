
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009-2011, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "file.h"

#include "debug.h"
#include "os.h"

#ifndef _MSC_VER
#  include <dirent.h>
#endif

#include <sys/stat.h>

namespace co
{
namespace base
{

Strings searchDirectory( const std::string& directory,
                         const std::string& pattern )
{
    Strings files;
   
#ifdef _MSC_VER
    WIN32_FIND_DATA file;
    const std::string search = 
        directory.empty() ? pattern : directory + '\\' + pattern;
    HANDLE hSearch = FindFirstFile( search.c_str(), &file );
    
    if( hSearch == INVALID_HANDLE_VALUE )
    {
        EQVERB << "Error finding the first file to match " << pattern << " in "
               << directory << std::endl;
        FindClose( hSearch );
        return files;
    }

    files.push_back( file.cFileName );    
    while( FindNextFile( hSearch, &file ))
        files.push_back( file.cFileName );    
    
    FindClose( hSearch );

#else

    const size_t findPos = pattern.find( '*' );
    if( findPos == std::string::npos )
    {
        EQASSERTINFO( 0, "Unimplemented" );
        return files;
    }

    const std::string first = pattern.substr( 0, findPos );
    const std::string second = pattern.substr( findPos + 1 );
  
    DIR* dir = opendir( directory.c_str() );
    if( dir == 0 )
    {
        EQVERB << "Can't open directory " << directory << std::endl;
        return files;
    }

    struct dirent* entry;
    
    while(( entry = readdir( dir )) != 0 )
    {
        const std::string candidate( entry->d_name );
        if( candidate.find( first ) != 0 )
            continue; // doesn't start with filename

        const size_t end = candidate.rfind( second );
        if( end == std::string::npos ||               // not found
            end + second.size() < candidate.size( ))  // not at the end
        {
            continue;
        }

        files.push_back( entry->d_name );  
    }
        
    closedir(dir);
#endif
    return files;
}

std::string getFilename( const std::string& filename )
{
    size_t lastSeparator = 0;
    const size_t length = filename.length();

    for( size_t i = 0; i < length; ++i )
        if( filename[ i ] == '/' || filename[i] == '\\' )
            lastSeparator = i+1;

    return filename.substr( lastSeparator, length );
}

std::string getDirname( const std::string& filename )
{
    size_t lastSeparator = 0;
    const size_t length = filename.length();

    for( size_t i = 0; i < length; ++i )
        if( filename[ i ] == '/' || filename[i] == '\\' )
            lastSeparator = i+1;

    return filename.substr( 0, lastSeparator );
}

}
}
