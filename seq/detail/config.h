
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

#ifndef EQSEQUEL_DETAIL_CONFIG_H
#define EQSEQUEL_DETAIL_CONFIG_H

#include <seq/types.h>
#include <eq/config.h> // base class
#include <eq/server.h> // RefPtr usage

namespace seq
{
namespace detail
{
class Config : public eq::Config
{
public:
    explicit Config( eq::ServerPtr parent )
        : eq::Config( parent ), _objects(0) {}

    seq::Application* getApplication();
    detail::Application* getApplicationImpl();

    virtual bool init() { LBDONTCALL; return false; }
    virtual bool run( co::Object* ) { LBDONTCALL; return false; }
    virtual bool exit() { LBDONTCALL; return false; }

    virtual bool needRedraw() { LBDONTCALL; return false; }
    virtual uint32_t startFrame() { LBDONTCALL; return 0; }

    virtual bool mapData( const uint128_t& )  { return true; }
    virtual void syncData( const uint128_t& ) { /* nop */ }
    virtual void unmapData() { /* nop */ }

    ObjectMap* getObjectMap();
    co::Object* getInitData();

protected:
    virtual ~Config() {}
    ObjectMap* _objects;

private:
};
}
}

#endif // EQSEQUEL_DETAIL_CONFIG_H
