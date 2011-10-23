
#ifndef EQ_RAW_CONVERTER
#define EQ_RAW_CONVERTER


namespace eVolve
{
    using std::string;
    
    class RawConverter
    {
    public:
        static int RawToRawPlusDerivativesConverter( const string& src,
                                                     const string& dst  );

        static int RawPlusDerivativesToRawConverter( const string& src,
                                                     const string& dst  );

        static int SavToVhfConverter(                const string& src,
                                                     const string& dst  );

        static int DscToVhfConverter(                const string& src,
                                                     const string& dst  );

        static int PvmSavToRawDerVhfConverter(       const string& src,
                                                     const string& dst  );

        static int CompareTwoRawDerVhf(              const string& src1,
                                                     const string& src2 );

        static int RecalculateDerivatives(           const string& src,
                                                     const string& dst );

        static int ScaleRawDerFile(                  const string& src,
                                                     const string& dst,
                                                           double scaleX,
                                                           double scaleY,
                                                           double scaleZ  );

        static int parseArguments( int argc, char** argv );
    };
}

#endif //EQ_RAW_CONVERTER
