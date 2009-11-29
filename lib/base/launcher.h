
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_LAUNCHER_H
#define EQBASE_LAUNCHER_H

#include <eq/base/base.h>
#include <string>
#include <vector>

namespace eq
{
namespace base
{
    /** The launcher executes a command from a separate process. */
    class Launcher
    {
    public:
        /** Execute the given command in a new process. @version 1.0 */
        static bool run( const std::string& command );

    private:
        Launcher(){}
#ifndef WIN32
        static void _buildCommandLine( const std::string& command,
                                       std::vector<std::string>& commandLine );
#endif
    };
}

}
#endif // EQBASE_LAUNCHER_H
