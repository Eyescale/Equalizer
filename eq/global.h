
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_GLOBAL_H
#define EQ_GLOBAL_H

#include <eq/api.h>
#include <eq/init.h>       // friend
#include <eq/types.h>

#include <eq/fabric/global.h>     // base class
#include <eq/fabric/iAttribute.h> // enum definition

namespace eq
{
/** Global parameter handling for the Equalizer client library. */
class Global : public fabric::Global
{
public:
    /** Set the name of the program. @version 1.5.2 */
    EQ_API static void setProgramName( const std::string& programName );

    /** @return the program name. @version 1.5.2 */
    EQ_API static const std::string& getProgramName();

    /** Set the working directory of the program. @version 1.5.2 */
    EQ_API static void setWorkDir( const std::string& workDir );

    /** @return the working directory of the program. @version 1.5.2 */
    EQ_API static const std::string& getWorkDir();

    /** @return the node factory. @version 1.0 */
    static NodeFactory* getNodeFactory() { return _nodeFactory; }

    /**
     * Set the config file or hwsd session for the app-local server.
     *
     * When started without specifying an explicit server connection, Equalizer
     * will create an server instance in an application thread using this
     * configuration. Strings ending with '.eqc' are considered Equalizer
     * configuration files, otherwise the given string is used as an HWSD
     * session name for auto-configuration.
     *
     * @param config the default configuration.
     * @version 1.0
     */
    EQ_API static void setConfig( const std::string& config );

    /** @return the configuration for the app-local server. @version 1.0 */
    EQ_API static const std::string& getConfig();

    /**
     * Global lock for all non-thread-safe Carbon API calls.
     *
     * Note: this is a NOP on non-AGL builds. Do not use unless you know the
     * side effects, i.e., ask on the eq-dev mailing list.
     *
     * @version 1.0
     */
    static void enterCarbon();

    /** Global unlock for non-thread-safe Carbon API calls. @version 1.0 */
    static void leaveCarbon();

private:
    EQ_API friend bool _init( const int argc, char** argv, NodeFactory* );
    EQ_API friend bool exit();

    static NodeFactory* _nodeFactory;
    static std::string  _config;
};
}

#endif // EQ_GLOBAL_H
