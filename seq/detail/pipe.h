
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

#ifndef EQSEQUEL_DETAIL_PIPE_H
#define EQSEQUEL_DETAIL_PIPE_H

#include <seq/types.h>
#include <eq/pipe.h> // base class

namespace seq
{
namespace detail
{
class Pipe : public eq::Pipe
{
public:
    explicit Pipe( eq::Node* parent );

    /** @name Data Access. */
    //@{
    seq::Application* getApplication();
    Config* getConfig();
    Node* getNode();
    seq::Renderer* getRenderer() { return _renderer; }
    detail::Renderer* getRendererImpl();
    co::Object* getFrameData();
    ObjectMap* getObjectMap();
    //@}

protected:
    virtual ~Pipe();

    virtual bool configInit( const uint128_t& initID );
    virtual bool configExit();

    virtual void frameStart( const uint128_t& frameID,
                             const uint32_t frameNumber );
private:
    bool _mapData( const uint128_t& initID );
    void _syncData( const uint128_t& version );
    void _unmapData();

    ObjectMap* _objects;
    seq::Renderer* _renderer;
};
}
}

#endif // EQSEQUEL_DETAIL_PIPE_H
