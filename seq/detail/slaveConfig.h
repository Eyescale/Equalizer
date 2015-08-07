
/* Copyright (c) 2011-2015, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQSEQUEL_DETAIL_SLAVECONFIG_H
#define EQSEQUEL_DETAIL_SLAVECONFIG_H

#include "config.h" // base class

namespace seq
{
namespace detail
{
class SlaveConfig : public Config
{
public:
    explicit SlaveConfig( eq::ServerPtr parent ) : Config( parent ) {}

    virtual bool mapData( const uint128_t& initID );
    virtual void syncData( const uint128_t& version );
    virtual void unmapData();

protected:
    virtual ~SlaveConfig() {}

private:
};
}
}

#endif // EQSEQUEL_DETAIL_SLAVECONFIG_H
