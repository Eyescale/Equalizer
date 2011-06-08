
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQSEQUEL_MODELMATRIX_H
#define EQSEQUEL_MODELMATRIX_H

#include <eq/sequel/api.h>
#include <eq/types.h>
#include <co/serializable.h>               // base class

namespace seq
{
    /** The main modelMatrix object. */
    class ModelMatrix : public co::Serializable
    {
    public:
        /** Construct a new model matrix. @version 1.0 */
        SEQ_API ModelMatrix();

        /** Destruct this model matrix. @version 1.0 */
        SEQ_API virtual ~ModelMatrix();

        /** @name Data Access. */
        //@{
        //@}

        /** @name Operations */
        //@{
        //@}

    protected:
        virtual void serialize( co::DataOStream& os, const uint64_t dirtyBits );
        virtual void deserialize( co::DataIStream& is,
                                  const uint64_t dirtyBits );

    private:
        /** The changed parts of the object since the last serialize(). */
        enum DirtyBits
        {
            DIRTY_MATRIX = co::Serializable::DIRTY_CUSTOM << 1, // 1
        };

        eq::Matrix4f _matrix;
    };
}
#endif // EQSEQUEL_MODELMATRIX_H
