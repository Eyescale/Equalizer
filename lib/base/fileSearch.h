/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
    #include <dirent.h>
#endif

namespace eq
{
namespace base
{
/* extract all files with the part name in the given directory*/
static StringVector fileSearch( const std::string directory,  
                                const std::string pattern )
{
    StringVector pluginDirectories;
   
#ifdef WIN32_VC

    WIN32_FIND_DATA file;
    
    SetCurrentDirectory ( directory.c_str() ); 

    HANDLE hSearch;
    bool find;
    
    hSearch=FindFirstFile( pattern.c_str(), &file );
    
    if(hSearch ==  INVALID_HANDLE_VALUE)
    {
        EQWARN << "Error to search file " 
                << std::endl;
        FindClose( hSearch );
        return;
    }

    find = true;
    do
    {   /* search and add files */
        std::string fichier = file.cFileName;
        pluginDirectories.push_back( fichier );    
        find = FindNextFile( hSearch, &file );
    } while(find);
    
    FindClose( hSearch );
#else

    // only foo*bar pattern are implemented
    const size_t findPos = data.find( '*' );
    
    if( findPos == string::npos )
    {
        EQWARN << "Error fileSearch foo*bar pattern invalid !!!!" 
                << std::endl;
        FindClose( hSearch );
        return;
    }
    const std::string first = pattern.substr( 0, findPos );
    const std::string second = pattern.substr( findPos + 1 );
  
    DIR* dir = opendir( path.c_str() );
    
    if( dir != 0 )
    {
        struct dirent* entry;
        
        while (( ent = readdir( rep )) != 0)
        {
            if(( strncmp( ent->d_name, first.c_str(), first.size() ) == 0) && 
               ( strcmp( ent->d_name, second.c_str() ) >= 0) )
            {
                std::string file( ent->d_name );
                pluginDirectories.push_back( file );  
            }   
        }
        
        closedir(rep);
     }

#endif
    return pluginDirectories;
}
}

}
#endif //EQBASE_EXECUTIONLISTENER_H
