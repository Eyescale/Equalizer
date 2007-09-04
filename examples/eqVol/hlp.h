
#ifndef EQ_HLP_H
#define EQ_HLP_H

namespace hlpFuncs
{
	
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
