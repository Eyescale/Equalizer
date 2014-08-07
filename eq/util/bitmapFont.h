
/* Copyright (c) 2008-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQUTIL_BITMAPFONT_H
#define EQUTIL_BITMAPFONT_H

#include <eq/client/api.h>
#include <eq/util/types.h>

namespace eq
{
namespace util
{
namespace detail { class BitmapFont; }

/** A wrapper around AGL, WGL and GLX bitmap fonts. */
class BitmapFont : public boost::noncopyable
{
public:
    /**
     * Construct a new bitmap font.
     *
     * Usually bitmap fonts are allocated using ObjectManager::newEqBitmapFont,
     * which provides the correct parameters for this constructor.
     *
     * @param gl An ObjectManager to allocate display lists for the fonts
     * @param key A unique key to allocate OpenGL objects from the object
     *            manager.
     * @version 1.0
     */
    EQ_API BitmapFont( ObjectManager& gl, const void* key );

    /**
     * Destruct this bitmap font.
     *
     * The destructor does not call exit, since it can't be sure there is a
     * valid OpenGL context current.
     * @version 1.0
     */
    EQ_API ~BitmapFont();

    /**
     * Initialize this bitmap font.
     *
     * The font name depends on the operating system. Please refer to your OS
     * tools to find available fonts. If no name is given, "Times New Roman" or
     * similar is used.
     *
     * An OpenGL context needs to be current, and eq::XGetCurrentDisplay() needs
     * to return the current display connection for GLX.
     *
     * @param ws the current window system.
     * @param name the name of the font.
     * @param size the size of the font in pixels.
     * @version 1.0
     */
    EQ_API bool init( const WindowSystem& ws, const std::string& name,
                      const uint32_t size = 12 );

    /** De-initialize this bitmap font. @version 1.0 */
    EQ_API void exit();

    /**
     * Draw text on the current raster position.
     * @version 1.0
     */
    EQ_API void draw( const std::string& text ) const;

private:
    detail::BitmapFont* const _impl;
};
}
}
#endif  // EQUTIL_BITMAPFONT_H
