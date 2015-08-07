
/* Copyright (c) 2009-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#include <eq/server/api.h>
#include "types.h"

#include <eq/fabric/observer.h>   // base class
#include <eq/fabric/eye.h>        // enum
#include <lunchbox/bitOperation.h> // function getIndexOfLastBit

#include <string>

namespace eq
{
namespace server
{
/** The observer. @sa eq::Observer */
class Observer : public fabric::Observer< Config, Observer >
{
public:
    /** Construct a new Observer. */
    EQSERVER_API explicit Observer( Config* parent );

    /** Destruct this observer. */
    virtual ~Observer();

    /** @name Data Access */
    //@{
    /** @return the Server of this observer. @version 1.0 */
    ServerPtr getServer();

    /** @return the position of an eye in world-space coordinates. */
    const fabric::Vector3f& getEyeWorld( const fabric::Eye eye ) const
    { return _eyeWorld[ lunchbox::getIndexOfLastBit( eye ) ]; }

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

    void addView( View* view );    //!< @internal
    void removeView( View* view ); //!< @internal

protected:
    virtual void setDirty( const uint64_t bits ); //!< @internal

    /** @sa Object::deserialize */
    virtual void deserialize( co::DataIStream& is,
                              const uint64_t dirtyBits );

private:
    /** Cached inverse head matrix. */
    fabric::Matrix4f _inverseHeadMatrix;

    /** The eye positions in world space. */
    fabric::Vector3f _eyeWorld[ eq::fabric::NUM_EYES ];

    /** Views tracked by this observer. */
    Views _views;

    enum State
    {
        STATE_ACTIVE = 0,  // next: DELETE
        STATE_DELETE,      // next: destructor
    }
        _state;

    struct Private;
    Private* _private; // placeholder for binary-compatible changes

    void _updateEyes();
    void _updateViews();
};
}
}
#endif // EQSERVER_OBSERVER_H
