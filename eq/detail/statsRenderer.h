
/* Copyright (c) 2012-2013, Stefan Eilemann <eile@eyescale>
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

#ifndef EQ_STATSRENDERER_H
#define EQ_STATSRENDERER_H

#include <GLStats/renderer.h>
#include <eq/util/bitmapFont.h>

namespace eq
{
namespace detail
{
    class StatsRenderer : public GLStats::Renderer
    {
    public:
        StatsRenderer( const util::BitmapFont* font )
                : GLStats::Renderer(), _font( font ) {}
        virtual ~StatsRenderer() {}

    protected:
        virtual void drawText( const std::string& text ) { _font->draw(text); }

    private:
        const util::BitmapFont* const _font;
    };
}
}

#endif // EQ_STATSRENDERER_H
