/* Copyright (c) 2007       Maxim Makhinya
   All rights reserved. */

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
