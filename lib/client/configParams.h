
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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
    /** Parameters for running a configuration. @sa Server::chooseConfig() */
    class ConfigParams
    {
    public:
        /** Construct new configuration parameters. @version 1.0 */
        EQ_EXPORT ConfigParams();

        /** Destruct this configuration parameters. @version 1.0 */
        ~ConfigParams(){}
        
        /** @name Data Access. */
        //@{
        /**
         * Set the name of the render client executable.
         *
         * The default value is the program name retrieved from
         * net::Global::getProgramName(), i.e., the filename part of argv[0].
         * @version 1.0
         */
        EQ_EXPORT void setRenderClient( const std::string& renderClient );

        /** @return the name of the render client executable. @version 1.0 */
        EQ_EXPORT const std::string& getRenderClient() const;

        /**
         * Set the directory from which to launch the render client.
         *
         * The default value is the program directory retrieved from
         * net::Global::getWorkDir(), i.e., the directory part of argv[0].
         * @version 1.0
         */
        EQ_EXPORT void setWorkDir( const std::string& workDir );

        /** 
         * @return the directory from which to launch the render client.
         * @version 1.0
         */
        EQ_EXPORT const std::string& getWorkDir() const;
        //@}

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

