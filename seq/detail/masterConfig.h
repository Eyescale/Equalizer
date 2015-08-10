
/* Copyright (c) 2011-2015, Stefan Eilemann <eile@eyescale.ch>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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


#ifndef EQSEQUEL_DETAIL_MASTERCONFIG_H
#define EQSEQUEL_DETAIL_MASTERCONFIG_H

#include "config.h" // base class

namespace seq
{
namespace detail
{
class MasterConfig : public Config
{
public:
    explicit MasterConfig( eq::ServerPtr parent );

    virtual bool init();
    virtual bool run( co::Object* frameData );
    virtual bool exit();

    virtual bool needRedraw() { return _redraw; }
    virtual uint32_t startFrame();

protected:
    virtual ~MasterConfig();
#ifndef EQ_2_0_API
    virtual bool handleEvent( const eq::ConfigEvent* event );
#endif
    virtual bool handleEvent( eq::EventICommand command );

private:
    uint128_t _currentViewID;
    bool _redraw;
};
}
}

#endif // EQSEQUEL_DETAIL_MASTERCONFIG_H
