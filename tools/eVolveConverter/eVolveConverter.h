
#ifndef EQ_RAW_CONVERTER
#define EQ_RAW_CONVERTER

#include <string>

namespace eVolve
{
    class RawConverter
    {
    public:
        static int RawToRawPlusDerivativesConverter( const std::string& src,
                                                     const std::string& dst  );

        static int RawPlusDerivativesToRawConverter( const std::string& src,
                                                     const std::string& dst  );

        static int SavToVhfConverter(                const std::string& src,
                                                     const std::string& dst  );

        static int DscToVhfConverter(                const std::string& src,
                                                     const std::string& dst  );

        static int PvmSavToRawDerVhfConverter(       const std::string& src,
                                                     const std::string& dst  );

        static int CompareTwoRawDerVhf(              const std::string& src1,
                                                     const std::string& src2 );

        static int RecalculateDerivatives(           const std::string& src,
                                                     const std::string& dst );

        static int ScaleRawDerFile(                  const std::string& src,
                                                     const std::string& dst,
                                                           double scaleX,
                                                           double scaleY,
                                                           double scaleZ  );

        static int parseArguments( int argc, char** argv );
    };
}

#endif //EQ_RAW_CONVERTER
