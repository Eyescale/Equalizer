
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

#ifndef EQSEQUEL_DETAIL_CHANNEL_H
#define EQSEQUEL_DETAIL_CHANNEL_H

#include <seq/types.h>
#include <eq/channel.h> // base class

namespace seq
{
namespace detail
{
class Channel : public eq::Channel
{
public:
    explicit Channel( eq::Window* parent );

    /** @name Data Access. */
    //@{
    Pipe* getPipe();
    const View* getView() const;
    const ViewData* getViewData() const;
    seq::Renderer* getRenderer();
    detail::Renderer* getRendererImpl();

    const Matrix4f& getViewMatrix() const { return getHeadTransform(); }
    const Matrix4f& getModelMatrix() const;

    virtual bool useOrtho() const;
    const RenderContext& getRenderContext() const { return getContext(); }
    //@}

    /** @name Operations. */
    //@{
    void applyRenderContext() { eq::Channel::frameDraw( uint128_t( )); }
    void applyModelMatrix();

    void clear() { return eq::Channel::frameClear( uint128_t( )); }
    //@}

protected:
    virtual ~Channel();

    virtual void frameStart( const uint128_t& frameID,
                             const uint32_t frameNumber );
    virtual void frameFinish( const uint128_t& frameID,
                              const uint32_t frameNumber );
    virtual void frameClear( const uint128_t& frameID );
    virtual void frameDraw( const uint128_t& frameID );
    virtual void frameViewFinish( const uint128_t& frameID );

private:
};
}
}

#endif // EQSEQUEL_DETAIL_CHANNEL_H
