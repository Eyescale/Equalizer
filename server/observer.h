
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_OBSERVER_H
#define EQSERVER_OBSERVER_H

#include "types.h"
#include "observerVisitor.h"    // used in inline method

#include <eq/client/observer.h> // base class
#include <string>

namespace eq
{
namespace server
{
    class ObserverPath;

    /**
     * The observer. @sa eq::Observer
     */
    class Observer : public eq::Observer
    {
    public:
        /** 
         * Constructs a new Observer.
         */
        Observer();

        /** Creates a new, deep copy of a observer. */
        Observer( const Observer& from, Config* config );

        /** Destruct this observer. */
        virtual ~Observer();

        /**
         * @name Data Access
         */
        //*{
        /** @return the index path to this observer. */
        ObserverPath getPath() const;
        //*}

        /**
         * @name Operations
         */
        //*{
        /** 
         * Traverse this observer using a observer visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( ObserverVisitor& visitor )
            { return visitor.visit( this ); }
        VisitorResult accept( ConstObserverVisitor& visitor ) const
            { return visitor.visit( this ); }

        /** Unmap this observer and all its children. */
        void unmap();
        //*}
        
    protected:

    private:
        /** The parent Config. */
        Config* _config;
        friend class Config;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };
}
}
#endif // EQSERVER_OBSERVER_H
