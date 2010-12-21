
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

#ifndef EQFABRIC_OBSERVER_H
#define EQFABRIC_OBSERVER_H

#include <eq/fabric/api.h>
#include <eq/fabric/object.h>         // base class
#include <eq/fabric/types.h>
#include <eq/fabric/visitorResult.h> // enum

#include <string>
#include <vector>

namespace eq
{
namespace fabric
{
    /** Base data transport class for observers. @sa eq::Observer */
    template< class C, class O > class Observer : public Object
    {
    public:
        /** The observer visitor type. @version 1.0 */
        typedef LeafVisitor< O > Visitor;

        /** @name Data Access */
        //@{
        /** @return the parent config of this observer. @version 1.0 */
        const C* getConfig() const { return _config; }

        /** @return the parent config of this observer. @version 1.0 */
        C* getConfig() { return _config; }

        /** @internal @return the index path to this observer. */
        ObserverPath getPath() const;

        /** Set the eye separation of this observer. @version 1.0 */
        EQFABRIC_INL void setEyeBase( const float eyeBase );

        /** @return the current eye separation. @version 1.0 */
        float getEyeBase() const { return _data.eyeBase; }

        /** 
         * Set the head matrix.
         *
         * The head matrix specifies the transformation origin->observer.
         * Together with the eye separation, this determines the eye positions
         * in the global coordinate system. The eye position and wall or
         * projection description define the shape of the frustum and the
         * channel's head transformation during rendering.
         *
         * @param matrix the matrix
         * @version 1.0
         */
        EQFABRIC_INL void setHeadMatrix( const Matrix4f& matrix );

        /** @return the current head matrix. @version 1.0 */
        const Matrix4f& getHeadMatrix() const { return _data.headMatrix; }
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
        EQFABRIC_INL Observer( C* config );

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
            DIRTY_EYE_BASE   = Object::DIRTY_CUSTOM << 0,
            DIRTY_HEAD       = Object::DIRTY_CUSTOM << 1,
            DIRTY_OBSERVER_BITS =
                DIRTY_EYE_BASE | DIRTY_HEAD | DIRTY_OBJECT_BITS
        };

        /** @internal @return the bits to be re-committed by the master. */
        virtual uint64_t getRedistributableBits() const
            { return DIRTY_OBSERVER_BITS; }

    private:
        /** The parent Config. */
        C* const _config;

        struct BackupData
        {
            BackupData() : eyeBase( .05f ), headMatrix( Matrix4f::IDENTITY ) {}

            /** The current eye separation. */
            float eyeBase;

            /** The current head position. */
            Matrix4f headMatrix;
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
