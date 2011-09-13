
/* Copyright (c)      2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_DSO_H
#define COBASE_DSO_H

#include <co/base/api.h>
#include <co/base/nonCopyable.h>

#include <string>

namespace co
{
namespace base
{
    class DSOPrivate;

    /** Helper to access dynamic shared objects (DSO) */
    class DSO : public NonCopyable
    {
    public:
        /** Construct a new dynamic shared object. @version 1.0 */
        COBASE_API DSO();

        /** Destruct this DSO handle. @version 1.0 */
        COBASE_API ~DSO();

        /** 
         * Open a dynamic shared object.
         * 
         * @param fileName The file name of the DSO.
         * @return true if the DSO was opened, false upon error.
         * @version 1.0
         */
        COBASE_API bool open( const std::string& fileName );

        /**
         * Close the DSO, invalidates retrieved function pointers.
         * @version 1.0
         */
        COBASE_API void close();
    
        /**
         * @return a function pointer in the DSO, or 0 if the function is not
         *         exported by the DSO.
         * @version 1.0
         */
        COBASE_API void* getFunctionPointer( const std::string& functionName );

        /** @return true if the DSO is loaded. @version 1.0 */
        COBASE_API bool isOpen() const;

    private:
        DSOPrivate* _data;
    };

}
}

#endif //COBASE_DSO_H
