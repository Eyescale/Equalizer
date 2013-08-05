
/* Copyright (c) 2006-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Daniel Pfeifer <daniel@pfeifer-mail.de>
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

#ifndef EQ_WINDOWSYSTEM_H
#define EQ_WINDOWSYSTEM_H

#include <eq/client/api.h>
#include <eq/client/types.h>

namespace eq
{

/**
 * The interface for windowing toolkits.
 *
 * This class is intended to be overwritten by an (anonymous) class with exactly
 * one static instance. The constructor will register this factory instance for
 * use with the WindowSystem frontend. See glx/windowSystem.cpp for an example.
 *
 * The implementation provides access to operating system dependent
 * functionality needed by Equalizer, and hides these specifics behind
 * interfaces.
 */
class WindowSystemIF
{
protected:
    /** Create a new window system instance. @version 1.6 */
    EQ_API WindowSystemIF();

    /** Destroy the window system instance. @version 1.6 */
    virtual ~WindowSystemIF() {}

    /** @internal */
    static uint32_t _setupLists( util::ObjectManager& gl, const void* key,
                                 const int num );

    /** @return the unique name of the window system. @version 1.6 */
    virtual std::string getName() const = 0;

    /** @return a new system pipe. @version 1.6 */
    virtual SystemPipe* createPipe(Pipe* pipe) const = 0;

    /** @return a new event message pump @version 1.6 */
    virtual MessagePump* createMessagePump() const = 0;

    /** @return a new system window @version 1.6 */
    virtual SystemWindow* createWindow(Window* window) const = 0;

    /**
     * Create a set of display lists for the given font.
     *
     * The implementation is expected to set up one display list per ASCII
     * character, and store the name of the first list in the given object
     * manager using the given key.
     *
     * @param gl the object manager for display list allocation.
     * @param key the key for display list allocation.
     * @param name the name of the font, OS-specific.
     * @param size the font size in points.
     * @return true if the font was created, false otherwise.
     * @warning experimental, might not be supported in the future.
     */
    virtual bool setupFont( util::ObjectManager& gl, const void* key,
                       const std::string& name, const uint32_t size ) const = 0;

    /** Perform per-process initialization for a Config. @version 1.6 */
    virtual void configInit( Node* /*node*/ ) {}

    /** Perform per-process de-initialization for a Config. @version 1.6 */
    virtual void configExit( Node* /*node*/ ) {}

private:
    WindowSystemIF* _next;
    friend class WindowSystem;
};

#ifndef EQ_2_0_API
enum WindowSystemEnum
{
    WINDOW_SYSTEM_NONE = 0, // must be first
    WINDOW_SYSTEM_AGL,      //!< AGL/Carbon
    WINDOW_SYSTEM_GLX,      //!< GLX/X11
    WINDOW_SYSTEM_WGL,      //!< WGL/Win32
    WINDOW_SYSTEM_ALL       // must be last
};
#endif

/** @internal
 * Access to the instantiated window systems.
 * @sa Pipe::getWindowSystem()
 */
class WindowSystem
{
public:
    EQ_API WindowSystem();
    EQ_API WindowSystem( const std::string& name );

#ifndef EQ_2_0_API
    EQ_API WindowSystem( const WindowSystemEnum type );
    ~WindowSystem() {}

    EQ_API bool operator == ( const WindowSystemEnum other ) const;
    EQ_API bool operator != ( const WindowSystemEnum other ) const;
    EQ_API operator WindowSystemEnum() const;
#endif

    static bool supports( std::string const& type );

    static void configInit( Node* node );
    static void configExit( Node* node );

    EQ_API std::string getName() const;

    EQ_API SystemWindow* createWindow( Window* window ) const;
    EQ_API SystemPipe* createPipe( Pipe* pipe ) const;
    EQ_API MessagePump* createMessagePump() const;
    EQ_API bool setupFont( util::ObjectManager& gl, const void* key,
                           const std::string& name, const uint32_t size ) const;

    EQ_API bool operator == ( const WindowSystem& other ) const;
    EQ_API bool operator != ( const WindowSystem& other ) const;

private:
    const WindowSystemIF* _impl;
    void _chooseImpl( const std::string& name );
};

/** Print the window system name to the given output stream. @version 1.0 */
EQ_API std::ostream& operator << ( std::ostream& os, const WindowSystem& );

/** co::Object serializer. @version 1.1.3 */
EQ_API co::DataOStream& operator << ( co::DataOStream& os, const WindowSystem& );

/** WindowSystem deserializer. @version 1.1.3 */
EQ_API co::DataIStream& operator >> ( co::DataIStream& is, WindowSystem& ws );

} // namespace eq

#endif // EQ_WINDOWSYSTEM_H
