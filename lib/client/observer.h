
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/client/object.h>         // base class
#include <eq/client/types.h>
#include <eq/client/visitorResult.h>  // enum

#include <string>

namespace eq
{
    class Config;
    class ObserverVisitor;

    /**
     * An Observer looks at one or more views from a certain position (head
     * matrix) with a given eye separation. Multiple observer in a configuration
     * can be used to update independent viewers from one configuration, e.g., a
     * control host, a HMD and a Cave.
     */
    class Observer : public Object
    {
    public:
        /** 
         * Constructs a new Observer.
         */
        EQ_EXPORT Observer();

        /** Destruct this observer. */
        EQ_EXPORT virtual ~Observer();

        /**
         * @name Data Access
         */
        //@{
        /** Set the eye separation of this observer. */
        EQ_EXPORT void setEyeBase( const float eyeBase );

        /** @return the current eye separation. */
        float getEyeBase() const { return _eyeBase; }

        /** 
         * Set the head matrix.
         *
         * The head matrix specifies the transformation origin->observer.
         * Together with the eye separation, this determines the eye positions.
         * The eye position and wall or projection description define the shape
         * of the frustum and the channel's head transformation during
         * rendering.
         *
         * @param matrix the matrix
         */
        EQ_EXPORT void setHeadMatrix( const Matrix4f& matrix );

        /** @return the current head matrix. */
        const Matrix4f& getHeadMatrix() const { return _headMatrix; }
        //@}

        /**
         * @name Operations
         */
        //@{
        /** 
         * Traverse this observer using a observer visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQ_EXPORT VisitorResult accept( ObserverVisitor& visitor );

        /** Deregister this observer from its net::Session.*/
        EQ_EXPORT virtual void deregister();
        //@}
        
    protected:
        /** @sa Object::serialize */
        EQ_EXPORT virtual void serialize( net::DataOStream& os,
                                          const uint64_t dirtyBits );
        /** @sa Object::deserialize */
        EQ_EXPORT virtual void deserialize( net::DataIStream& is, 
                                            const uint64_t dirtyBits );

        enum DirtyBits
        {
            DIRTY_EYE_BASE   = Object::DIRTY_CUSTOM << 0,
            DIRTY_HEAD       = Object::DIRTY_CUSTOM << 1,
            DIRTY_FILL1      = Object::DIRTY_CUSTOM << 2,
            DIRTY_FILL2      = Object::DIRTY_CUSTOM << 3,
            DIRTY_CUSTOM     = Object::DIRTY_CUSTOM << 4
        };

    private:
        /** The parent Config. */
        Config* _config;
        friend class Config;

        /** The current eye separation. */
        float _eyeBase;

        /** The current head position. */
        Matrix4f _headMatrix;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream&, const Observer* );
}
#endif // EQ_OBSERVER_H
