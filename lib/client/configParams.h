
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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

