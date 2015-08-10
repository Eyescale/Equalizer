
#ifndef EQ_HLP_H
#define EQ_HLP_H

#include <iostream>
#include <fstream>

namespace hlpFuncs
{

/** Just helping structure to automatically
    close files
*/
struct hFile
{
    hFile() : f( 0 ) {}
    explicit hFile( FILE *file ) : f( file ) {}
    ~hFile() { if( f ) fclose( f ); }

    FILE *f;
};

template <class T>
T min( T a, T b )
{
    return (a < b ? a : b);
}

template <class T>
T max( T a, T b )
{
    return (a > b ? a : b);
}

template <class T>
T clip( T val, T min, T max )
{
    return ( val<min ? min : ( val>max ? max : val ) );
}

}

#endif //EQ_HLP_H
