
/* Copyright (c) 2014, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_WINDOW_SETTINGS_H
#define EQ_WINDOW_SETTINGS_H

#include <eq/api.h>
#include <eq/types.h>
#include <eq/fabric/windowSettings.h> // base class


namespace eq
{
namespace detail { class WindowSettings; }

/** A set of settings to setup an eq::SystemWindow. */
class WindowSettings : public fabric::WindowSettings
{
public:
    /** Create a new WindowSettings. @version 1.7.2 */
    EQ_API WindowSettings();

    /** Destroy the WindowSettings. @version 1.7.2 */
    EQ_API ~WindowSettings();

    /** @internal */
    EQ_API WindowSettings( const WindowSettings& rhs );

    /** @internal */
    EQ_API WindowSettings& operator=( const WindowSettings& rhs );

    /** @sa eq::Window::setSharedContextWindow() @version 1.7.2 */
    EQ_API void setSharedContextWindow( const SystemWindow* window );

    /** @sa eq::Window::getSharedContextWindow() @version 1.7.2 */
    EQ_API const SystemWindow* getSharedContextWindow() const;

    /**
     * @return the OpenGL texture format corresponding to the window's color
     *         drawable configuration
     * @version 1.7.2
     */
    EQ_API uint32_t getColorFormat() const;

private:
    detail::WindowSettings* const _impl;
};
}


#endif // EQ_WINDOW_SETTINGS_H
