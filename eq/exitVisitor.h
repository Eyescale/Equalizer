
/* Copyright (c) 2013-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <eq/fabric/configVisitor.h>

namespace eq
{
namespace detail
{
class ExitVisitor : public ConfigVisitor
{
public:
    VisitorResult visit( eq::Observer* observer ) final
    {
        if( observer->configExit( ))
            return TRAVERSE_CONTINUE;
        LBWARN << *observer << " exit failed" << std::endl;
        return TRAVERSE_TERMINATE;
    }

    VisitorResult visit( eq::View* view ) final
    {
        if( view->configExit( ))
            return TRAVERSE_CONTINUE;
        LBWARN << *view << " exit failed" << std::endl;
        return TRAVERSE_TERMINATE;
    }
};
}
}
