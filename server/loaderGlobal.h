
 
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADER_GLOBAL_H
#define EQS_LOADER_GLOBAL_H

#include "loaderConnection.h"

#include <boost/spirit/core.hpp>

namespace eqLoader
{
    using namespace boost::spirit;
    using namespace eqs;
    using namespace std;

    struct setGlobalConnectionSAttr
    {
        setGlobalConnectionSAttr( const ConnectionDescription::SAttribute attr )
                : _attr( attr ) {}

        template <typename IteratorT>

        void operator()(IteratorT first, IteratorT last) const
            {
                string result;
                while (first != last)
                    result += *first++;
                
                Global::instance()->setConnectionSAttribute( _attr, result );
            }
    
        ConnectionDescription::SAttribute _attr;
    };

    struct setGlobalConnectionIAttr
    {
        setGlobalConnectionIAttr( const ConnectionDescription::IAttribute attr )
                : _attr( attr ) {}

        template <typename IteratorT>

        void operator()( const unsigned value ) const
            {
                EQINFO << "setGlobalConnectionIAttr " << _attr <<  " = "
                       << value << endl;

                Global::instance()->setConnectionIAttribute( _attr, value );
            }
        void operator()( const int value ) const
            {
                Global::instance()->setConnectionIAttribute( _attr, value );
            }
    
        ConnectionDescription::IAttribute _attr;
    };

    struct GlobalGrammar : public grammar<GlobalGrammar>
    {
        GlobalGrammar() {}

        template <typename ScannerT> struct definition
        {
            rule<ScannerT> const& start() const 
                { return global; }

            definition( GlobalGrammar const& self )
                {
                    global = "global" 
                        >> ch_p('{')
                        >> *(
                            ( str_p("EQ_CONNECTION_TYPE") 
                              >> connectionType_p[setGlobalConnectionType()] )
                        |   ( str_p("EQ_CONNECTION_TCPIP_PORT")
                              >> +(uint_p)[ setGlobalConnectionIAttr( 
                                      ConnectionDescription::IATTR_TCPIP_PORT)])
                        |   ( str_p("EQ_CONNECTION_LAUNCH_TIMEOUT")
                              >> +(uint_p)[ setGlobalConnectionIAttr( 
                                  ConnectionDescription::IATTR_LAUNCH_TIMEOUT)]
                              >> !str_p("ms") )
                        |   ( str_p("EQ_CONNECTION_HOSTNAME")
                              >> ch_p('"')
                              >> (+(alnum_p))[ setGlobalConnectionSAttr( 
                                      ConnectionDescription::SATTR_HOSTNAME ) ]
                              >> ch_p('"') )
                        |    ( str_p("EQ_CONNECTION_LAUNCH_COMMAND")
                               >> ch_p('"')
                               >> (+(anychar_p - ch_p('"') ))[
                                   setGlobalConnectionSAttr( 
                                  ConnectionDescription::SATTR_LAUNCH_COMMAND)]
                               >> ch_p('"'))
                            )
                        >> ch_p('}');
                }

            rule<ScannerT> global;
        };
    } 
        global_g;

//     inline GlobalGrammar global_p()
//     { 
//         return GlobalGrammar();
//     }
}

#endif // EQS_LOADER_GLOBAL_H
