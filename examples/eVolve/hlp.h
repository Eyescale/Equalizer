
/* Copyright (c) 2007       Maxim Makhinya
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

#ifndef EVOLVE_HLP_H
#define EVOLVE_HLP_H

#include <iostream>
#include <fstream>

namespace hlpFuncs
{

/** Just helping structure to automatically
    close files
*/
struct hFile
{
    hFile()             : f(NULL) {}
    hFile( FILE *file ) : f(file) {}
    ~hFile() { if( f ) fclose( f ); }

    FILE *f;
};

template <class T>
T clip( T val, T min, T max )
{
    return ( val<min ? min : ( val>max ? max : val ) );
}

}

#endif // EVOLVE_HLP_H
