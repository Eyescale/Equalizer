
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQSERVER_OBSERVER_H
#define EQSERVER_OBSERVER_H

#include "types.h"

#include <eq/fabric/observer.h>   // base class
#include <eq/fabric/eye.h>        // enum
#include <co/base/bitOperation.h> // function getIndexOfLastBit

#include <string>

namespace eq
{
namespace server
{
    /** The observer. @sa eq::Observer */
    class Observer : public fabric::Observer< Config, Observer >
    {
    public:
        /** 
         * Constructs a new Observer.
         */
        EQSERVER_EXPORT Observer( Config* parent );

        /** Destruct this observer. */
        virtual ~Observer();

        /** @name Data Access */
        //@{
        /** @return the Server of this observer. @version 1.0 */
        ServerPtr getServer();

        /** @return the position of an eye in world-space coordinates. */
        const fabric::Vector3f& getEyePosition( const fabric::Eye eye ) const
        { return _eyes[ co::base::getIndexOfLastBit( eye ) ]; }

        /** @return the inverse of the current head matrix. */
        const fabric::Matrix4f& getInverseHeadMatrix() const
            { return _inverseHeadMatrix; }

        /** @return true if this observer should be deleted. */
        bool needsDelete() const { return _state == STATE_DELETE; }
        //@}

        /**
         * @name Operations
         */
        //@{
        /** Initialize the observer parameters. */
        void init();

        /** Schedule deletion of this observer. */
        void postDelete();
        //@}
        
    protected:
        /** @sa Object::deserialize */
        virtual void deserialize( co::DataIStream& is, 
                                  const uint64_t dirtyBits );

    private:
        /** Cached inverse head matrix. */
        fabric::Matrix4f _inverseHeadMatrix;

        /** The eye positions in world space. */ 
        fabric::Vector3f _eyes[ eq::fabric::NUM_EYES ];

        enum State
        {
            STATE_ACTIVE = 0,  // next: DELETE
            STATE_DELETE,      // next: destructor
        }
            _state;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        void _updateEyes();
    };
}
}
#endif // EQSERVER_OBSERVER_H
