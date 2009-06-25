/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_FILESEARCH_H
#define EQBASE_FILESEARCH_H

#ifndef WIN32_VC
#  include <dirent.h>
#endif

namespace eq
{
namespace base
{

/* @return all file names matching the given pattern in the given directory*/
inline StringVector fileSearch( const std::string directory,  
                                const std::string pattern )
{
    StringVector files;
   
#ifdef WIN32_VC

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

    bool found = true;
    do
    {   /* search and add files */
        files.push_back( file.cFileName );    
        found = FindNextFile( hSearch, &file );
    }
    while( found );
    
    FindClose( hSearch );
#else

    // only foo*bar pattern are implemented
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
    
    while (( entry = readdir( dir )) != 0)
    {
        if(( strncmp( entry->d_name, first.c_str(), first.size() ) == 0) && 
           ( strcmp( entry->d_name, second.c_str() ) >= 0) )
        {
            files.push_back( entry->d_name );  
        }   
    }
        
    closedir(dir);
#endif
    return files;
}

inline std::string getBasename( const std::string& filename )
{
    size_t lastSeparator = 0;
    const size_t length = filename.length();

    for( size_t i = 0; i < length; ++i )
        if( filename[ i ] == '/' || filename[i] == '\\' )
            lastSeparator = i+1;

    return filename.substr( lastSeparator, length );
}

inline std::string getDirname( const std::string& filename )
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
#endif //EQBASE_FILESEARCH_H
