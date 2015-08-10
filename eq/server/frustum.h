
/* Copyright (c) 2008-2015, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQSERVER_FRUSTUM_H
#define EQSERVER_FRUSTUM_H

#include <eq/fabric/frustum.h>     // base class

namespace eq
{
namespace server
{
class FrustumData;

/** Extends fabric::Frustum to update server-side generic frustum data. */
class Frustum : public fabric::Frustum
{
public:
    explicit Frustum( FrustumData& data );
    Frustum( const Frustum& from, FrustumData& data );
    virtual ~Frustum(){}

private:
    FrustumData& _data;

    /** Update the frustum (wall/projection). */
    virtual void updateFrustum();
    virtual void notifyFrustumChanged() { updateFrustum(); }
};
}
}
#endif // EQ_FRUSTUM_H
