
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_OBSERVER_H
#define EQ_OBSERVER_H

#include <eq/fabric/observer.h>         // base class
#include <eq/client/types.h>

namespace eq
{
    class Config;

    /**
     * An Observer looks at one or more views from a certain position (head
     * matrix) with a given eye separation. Multiple observer in a configuration
     * can be used to update independent viewers from one configuration, e.g., a
     * control host, a HMD and a Cave.
     */
    class Observer : public fabric::Observer< Config, Observer >
    {
    public:
        /** Construct a new observer. @version 1.0 */
        EQ_EXPORT Observer( Config* parent );

        /** Destruct this observer. @version 1.0 */
        EQ_EXPORT virtual ~Observer();

        /** @name Data Access */
        //@{
        /** @return the Server of this observer. @version 1.0 */
        EQ_EXPORT ServerPtr getServer();
        //@}

    private:
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}
#endif // EQ_OBSERVER_H
