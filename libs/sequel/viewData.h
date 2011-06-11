
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

#ifndef EQSEQUEL_VIEWDATA_H
#define EQSEQUEL_VIEWDATA_H

#include <eq/sequel/api.h>
#include <eq/sequel/types.h>
#include <co/serializable.h>               // base class

namespace seq
{
    /** The main viewData object. */
    class ViewData : public co::Serializable
    {
    public:
        /** Construct a new view data. @version 1.0 */
        SEQ_API ViewData();

        /** Destruct this view data. @version 1.0 */
        SEQ_API virtual ~ViewData();

        /** @name Data Access. */
        //@{
        //@}

        /** @name Operations */
        //@{
        void spinModel( const float x, const float y, const float z );
        void moveModel( const float x, const float y, const float z );
        //@}

    protected:
        virtual void serialize( co::DataOStream& os, const uint64_t dirtyBits );
        virtual void deserialize( co::DataIStream& is,
                                  const uint64_t dirtyBits );

    private:
        /** The changed parts of the object since the last serialize(). */
        enum DirtyBits
        {
            DIRTY_MODELMATRIX = co::Serializable::DIRTY_CUSTOM << 1, // 1
        };

        Matrix4f _modelMatrix;
    };
}
#endif // EQSEQUEL_VIEWDATA_H
