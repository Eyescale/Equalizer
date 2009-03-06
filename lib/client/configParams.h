
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIG_PARAMS_H
#define EQ_CONFIG_PARAMS_H

#include <eq/base/base.h>
#include <string>

namespace eq
{
    class EQ_EXPORT ConfigParams
    {
    public:
        ConfigParams();
        virtual ~ConfigParams(){}

        ConfigParams& operator = ( const ConfigParams& rhs );

        void setRenderClient( const std::string& renderClient );
        const std::string& getRenderClient() const;

        void setWorkDir( const std::string& workDir );
        const std::string& getWorkDir() const;

    private:
        std::string _renderClient;
        std::string _workDir;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };
}

#endif // EQ_CONFIG_PARAMS_H

