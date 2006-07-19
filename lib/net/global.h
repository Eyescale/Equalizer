
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_GLOBAL_H
#define EQNET_GLOBAL_H

#include <string>

namespace eqNet
{
    // global defines

    /** 
     * Global parameter handling for the Equalizer network implementation. 
     */
    class Global
    {
    public:
        /** 
         * Sets the name of the program.
         * 
         * @param programName the program name.
         */
        static void setProgramName( const std::string& programName )
            { _programName = programName; }

        /** 
         * Gets the name of the program.
         * 
         * @return the program name.
         */
        static const std::string& getProgramName() { return _programName; }

        /** 
         * Sets the working directory of the program.
         * 
         * @param workDir the working directory.
         */
        static void setWorkDir( const std::string& workDir )
            { _workDir = workDir; }

        /** 
         * Gets the working directory of the program.
         * 
         * @return the working directory.
         */
        static const std::string& getWorkDir() { return _workDir; }

    private:
        static std::string _programName;
        static std::string _workDir;
    };
}

#endif // EQNET_GLOBAL_H

