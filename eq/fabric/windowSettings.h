
/* Copyright (c) 2014-2015, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQFABRIC_WINDOW_SETTINGS_H
#define EQFABRIC_WINDOW_SETTINGS_H

#include <eq/fabric/api.h>
#include <eq/fabric/pixelViewport.h>


namespace eq
{
namespace fabric
{
namespace detail { class WindowSettings; }

/** A set of settings to setup an eq::fabric::Window. */
class WindowSettings
{
public:
    /** Create a new WindowSettings. @version 1.7.2 */
    EQFABRIC_API WindowSettings();

    /** Destroy the WindowSettings. @version 1.7.2 */
    EQFABRIC_API virtual ~WindowSettings();

    /** @internal */
    EQFABRIC_API WindowSettings( const WindowSettings& rhs );

    /** @internal */
    EQFABRIC_API WindowSettings& operator=( const WindowSettings& rhs );

    /**
     * Window attributes.
     *
     * Most of these attributes are used by the SystemWindow implementation to
     * configure the window during configInit(). A SystemWindow implementation
     * might not respect all attributes, e.g., IATTR_HINT_SWAPSYNC is not
     * implemented by the GLXWindow. Please refer to the Programming Guide for
     * details.  @version 1.0
     */
    enum IAttribute
    {
        // Note: also update string array initialization in window.ipp
        IATTR_HINT_CORE_PROFILE,     //!< Core profile context if possible
        IATTR_HINT_OPENGL_MAJOR,     //!< Major version for GL context creation
        IATTR_HINT_OPENGL_MINOR,     //!< Minor version for GL context creation
        IATTR_HINT_STEREO,           //!< Active stereo
        IATTR_HINT_DOUBLEBUFFER,     //!< Front and back buffer
        IATTR_HINT_FULLSCREEN,       //!< Fullscreen drawable
        IATTR_HINT_DECORATION,       //!< Window decorations
        IATTR_HINT_SWAPSYNC,         //!< Swap sync on vertical retrace
        IATTR_HINT_DRAWABLE,         //!< Window, pbuffer, FBO or OFF
        IATTR_HINT_STATISTICS,       //!< Statistics gathering hint
        IATTR_HINT_SCREENSAVER,      //!< Screensaver (de)activation (WGL)
        IATTR_HINT_GRAB_POINTER,     //!< Capture mouse outside window
        IATTR_HINT_WIDTH,            //!< Default horizontal resolution
        IATTR_HINT_HEIGHT,           //!< Default vertical resolution
        IATTR_PLANES_COLOR,          //!< No of per-component color planes
        IATTR_PLANES_ALPHA,          //!< No of alpha planes
        IATTR_PLANES_DEPTH,          //!< No of z-buffer planes
        IATTR_PLANES_STENCIL,        //!< No of stencil planes
        IATTR_PLANES_ACCUM,          //!< No of accumulation buffer planes
        IATTR_PLANES_ACCUM_ALPHA,    //!< No of alpha accum buffer planes
        IATTR_PLANES_SAMPLES,        //!< No of multisample (AA) planes
        IATTR_LAST,
        IATTR_ALL = IATTR_LAST
    };

    /**
     * Set a window attribute.
     *
     * @return true if the attribute was changed, false otherwise.
     * @version 1.7.2
     */
    EQFABRIC_API bool setIAttribute( const IAttribute attr,
                                     const int32_t value );

    /** @return the value of a window attribute. @version 1.7.2 */
    EQFABRIC_API int32_t getIAttribute( const IAttribute attr ) const;

    /**
     * @return the window's pixel viewport wrt the parent pipe.
     * @version 1.7.2
     */
    EQFABRIC_API const PixelViewport& getPixelViewport() const;

    /**
     * Set the window's pixel viewport wrt its parent pipe.
     *
     * @param pvp the viewport in pixels.
     * @version 1.7.2
     */
    EQFABRIC_API void setPixelViewport( const PixelViewport& pvp );

    /**  @return the window's name. @version 1.7.2 */
    EQFABRIC_API const std::string& getName() const;

    /** Set the window's name. @version 1.7.2 */
    EQFABRIC_API void setName( const std::string& name );

    /** @internal */
    EQFABRIC_API void serialize( co::DataOStream& os ) const;

    /** @internal */
    EQFABRIC_API void deserialize( co::DataIStream& is );

private:
    detail::WindowSettings* const _impl;
};
}
}


#endif // EQFABRIC_WINDOW_SETTINGS_H
