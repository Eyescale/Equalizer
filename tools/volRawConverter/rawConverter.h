
#ifndef EQ_RAW_CONVERTER
#define EQ_RAW_CONVERTER


namespace eVolve
{
    using std::string;
    
    class RawConverter
    {
    public:
        static void ConvertRawToRawPlusDerivatives( const string& src,
                                                    const string& dst  );
                                                    
        static void SavToVhfConverter(              const string& src,
                                                    const string& dst  );
                                                    
        static void DscToVhfConverter(              const string& src,
                                                    const string& dst  );

        static void RawConverter::parseArguments( int argc, char** argv );
    };

}

#endif //EQ_RAW_CONVERTER
