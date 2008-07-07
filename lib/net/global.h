
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_GLOBAL_H
#define EQNET_GLOBAL_H

#include <eq/base/base.h>
#include <string>

namespace eq
{
namespace net
{
    // global defines

    /** 
     * Global parameter handling for the Equalizer network implementation. 
     */
    class EQ_EXPORT Global
    {
    public:
        /** 
         * Sets the name of the program.
         * 
         * @param programName the program name.
         */
        static void setProgramName( const std::string& programName );

        /** @return the program name. */
        static const std::string& getProgramName() { return _programName; }

        /** 
         * Sets the working directory of the program.
         * 
         * @param workDir the working directory.
         */
        static void setWorkDir( const std::string& workDir );

        /** @return the working directory of the program. */
        static const std::string& getWorkDir() { return _workDir; }


        /** 
         * Sets the default listening port.
         * 
         * @param port the default port.
         */
        static void setDefaultPort( const uint16_t port ) 
            { _defaultPort = port; }

        /** @return the default listening port. */
        static uint16_t getDefaultPort() { return _defaultPort; }

    private:
        static std::string _programName;
        static std::string _workDir;
        static uint16_t    _defaultPort;
    };
}
}

#endif // EQNET_GLOBAL_H

