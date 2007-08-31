
#ifndef EQ_RAW_CONVERTER
#define EQ_RAW_CONVERTER

#include <eq/eq.h>

namespace eqVol
{

using namespace std;
    
class RawConverter
{
public:
    static void ConvertRawToRawPlusDerivatives( const string& src, const string& dst );
    static void SavToVhfConverter(              const string& src, const string& dst );
    static void DscToVhfConverter(              const string& src, const string& dst );
};

}

#endif //EQ_RAW_CONVERTER
