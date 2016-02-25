
/* Copyright (c) 2009-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQFABRIC_OBSERVER_H
#define EQFABRIC_OBSERVER_H

#include <eq/fabric/api.h>
#include <eq/fabric/eye.h>           // enum
#include <eq/fabric/focusMode.h>     // enum
#include <eq/fabric/object.h>        // base class
#include <eq/fabric/vmmlib.h>
#include <string>
#include <vector>

namespace eq
{
namespace fabric
{
/** Base data transport class for observers. @sa eq::Observer */
// cppcheck-suppress noConstructor
template< class C, class O > class Observer : public Object
{
public:
    /** The observer visitor type. @version 1.0 */
    typedef LeafVisitor< O > Visitor;

    /** @name Data Access */
    //@{
    /**
     * Set the head matrix.
     *
     * The head matrix specifies the transformation origin->observer.  Together
     * with the eye separation, this determines the eye positions in the global
     * coordinate system. The eye position and wall or projection description
     * define the shape of the frustum and the channel's head transformation
     * during rendering.
     *
     * @param matrix the matrix
     * @return true if the matrix was changed, false otherwise.
     * @version 1.0
     */
    EQFABRIC_INL bool setHeadMatrix( const Matrix4f& matrix );

    /** @return the current head matrix. @version 1.0 */
    const Matrix4f& getHeadMatrix() const { return _data.headMatrix; }

    /**
     * Set the position of the given eye relative to the observer.
     *
     * A standard symmetric eye setup uses (+-eyeBase/2, 0, 0).
     * @param eye the eye to update.
     * @param pos the new eye position.
     * @version 1.5.2
     */
    EQFABRIC_INL void setEyePosition( const Eye eye, const Vector3f& pos );

    /** @return the position of the given eye. @version 1.5.2 */
    EQFABRIC_INL const Vector3f& getEyePosition( const Eye eye ) const;


    /** Set the focal distance. @sa setFocusMode @version 1.1 */
    EQFABRIC_INL void setFocusDistance( const float focusDistance );

    /** @return the current focal distance. @version 1.1 */
    float getFocusDistance() const { return _data.focusDistance; }

    /** Set the focal mode. @version 1.1 */
    EQFABRIC_INL void setFocusMode( const FocusMode focusMode );

    /** @return the current focal mode. @version 1.1 */
    FocusMode getFocusMode() const { return _data.focusMode; }

    /** Set the index of the OpenCV camera for tracking. @version 1.5.2 */
    EQFABRIC_INL void setOpenCVCamera( const int32_t index );

    /** @return the current OpenCV camera. @version 1.5.2 */
    int32_t getOpenCVCamera() const { return _data.camera; }

    /** Set the VRPN tracker device. @version 1.5.2 */
    EQFABRIC_INL void setVRPNTracker( const std::string& index );

    /** @return the current VRPN tracker device name. @version 1.5.2 */
    const std::string& getVRPNTracker() const { return _data.vrpnTracker; }

    /** @return the parent config of this observer. @version 1.0 */
    const C* getConfig() const { return _config; }

    /** @return the parent config of this observer. @version 1.0 */
    C* getConfig() { return _config; }

    /** @internal @return the index path to this observer. */
    ObserverPath getPath() const;
    //@}

    /** @name Operations */
    //@{
    /**
     * Traverse this observer using a observer visitor.
     *
     * @param visitor the visitor.
     * @return the result of the visitor traversal.
     * @version 1.0
     */
    EQFABRIC_INL VisitorResult accept( Visitor& visitor );

    /** Const-version of accept(). @version 1.0 */
    EQFABRIC_INL VisitorResult accept( Visitor& visitor ) const;

    virtual void backup(); //!< @internal
    virtual void restore(); //!< @internal
    //@}

protected:
    /** @internal Construct a new Observer. */
    EQFABRIC_INL explicit Observer( C* config );

    /** @internal Destruct this observer. */
    EQFABRIC_INL virtual ~Observer();

    /** @internal */
    virtual void serialize( co::DataOStream& os,
                            const uint64_t dirtyBits );
    /** @internal */
    virtual void deserialize( co::DataIStream& is,
                              const uint64_t dirtyBits );
    virtual void setDirty( const uint64_t bits ); //!< @internal

    /** @internal */
    enum DirtyBits
    {
        DIRTY_EYE_POSITION = Object::DIRTY_CUSTOM << 0,
        DIRTY_HEAD         = Object::DIRTY_CUSTOM << 1,
        DIRTY_FOCUS        = Object::DIRTY_CUSTOM << 2,
        DIRTY_TRACKER      = Object::DIRTY_CUSTOM << 3,
        DIRTY_OBSERVER_BITS =
        DIRTY_EYE_POSITION | DIRTY_HEAD | DIRTY_FOCUS | DIRTY_TRACKER |
        DIRTY_OBJECT_BITS
    };

    /** @internal @return the bits to be re-committed by the master. */
    virtual uint64_t getRedistributableBits() const
    { return DIRTY_OBSERVER_BITS; }

private:
    /** The parent Config. */
    C* const _config;

    struct BackupData
    {
        BackupData();

        Matrix4f headMatrix; //!< The current head position
        Vector3f eyePosition[ NUM_EYES ]; //!< The current eye positions
        float focusDistance; //!< The current focal distance
        FocusMode focusMode; //!< The current focal distance mode
        int32_t camera; //!< The OpenCV camera used for head tracking
        std::string vrpnTracker; //!< VRPN tracking device
    }
        _data, _backup;

    struct Private;
    Private* _private; // placeholder for binary-compatible changes
};

template< class C, class O >
EQFABRIC_INL std::ostream& operator << ( std::ostream&,
                                         const Observer< C, O >& );
}
}
#endif // EQFABRIC_OBSERVER_H
