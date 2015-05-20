
/* Copyright (c) 2011-2015, Stefan Eilemann <eile@eyescale.ch>
 *                          Petros Kataras <petroskataras@gmail.com>
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

#include "config.h"

#include "application.h"
#include "objectMap.h"

#include <seq/application.h>

namespace seq
{
namespace detail
{
seq::Application* Config::getApplication()
{
    return static_cast< seq::Application* >( getClient().get( ));
}

detail::Application* Config::getApplicationImpl()
{
    return getApplication()->getImpl();
}

co::Object* Config::getInitData()
{
    LBASSERT( _objects );
    if( !_objects )
        return 0;

    co::Object* initData = getApplicationImpl()->getInitData();
    return _objects->getInitData( initData );
}

ObjectMap* Config::getObjectMap()
{
    return _objects;
}

}
}
