
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

#ifndef EQSEQUEL_DETAIL_VIEW_H
#define EQSEQUEL_DETAIL_VIEW_H

#include <eq/view.h> // base class
#include <eq/sequel/types.h>

namespace seq
{
namespace detail
{
    class View : public eq::View
    {
    public:
        View( eq::Layout* parent );

        /** @name Data Access. */
        //@{
        Config* getConfig();
        Pipe* getPipe();
        ViewData* getViewData();
        const ViewData* getViewData() const;
        //@}

        /** @name Operations. */
        //@{
        bool handleEvent( const eq::ConfigEvent* event );
        bool updateData();
        //@}

    protected:
        virtual ~View();
        virtual void notifyAttach();
        virtual void notifyDetached();

    private:
        int _spinX, _spinY;
        int _advance;
    };
}
}

#endif // EQSEQUEL_DETAIL_VIEW_H
