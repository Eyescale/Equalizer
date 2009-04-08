
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

#include <eq/client/eye.h>      // enum
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

        /** @return the position of an eye in world-space coordinates. */
        const vmml::Vector3f& getEyePosition( const eq::Eye eye ) const
            { return _eyes[ eye ]; }

        /** @return the inverse of the current head matrix. */
        const vmml::Matrix4f& getInverseHeadMatrix() const
            { return _inverseHeadMatrix; }

        /** @return the config of this observer. */
        Config* getConfig() { return _config; }
        //*}

        /**
         * @name Operations
         */
        //*{
        /** Initialize the observer parameters. */
        void init();

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
        /** @sa Object::deserialize */
        virtual void deserialize( net::DataIStream& is, 
                                  const uint64_t dirtyBits );

    private:
        /** The parent Config. */
        Config* _config;
        friend class Config;

        /** Cached inverse head matrix. */
        vmml::Matrix4f _inverseHeadMatrix;

        /** The eye positions in world space. */ 
        vmml::Vector3f _eyes[eq::EYE_ALL];

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        void _updateEyes();
    };
}
}
#endif // EQSERVER_OBSERVER_H
