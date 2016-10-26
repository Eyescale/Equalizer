
/* Copyright (c) 2016, Stefan.Eilemann@epfl.ch
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

#ifndef EQFABRIC_KEYEVENT_H
#define EQFABRIC_KEYEVENT_H

#include <eq/fabric/event.h>

namespace eq
{
namespace fabric
{
/** Event for a key press or release.  */
struct KeyEvent : public Event
{
    KeyEvent( const uint32_t k = 0 ) : key( k ) {}

    uint32_t key; //!<  KeyCode for special keys, ascii code otherwise
    uint32_t modifiers; ; //!< key modifier mask
};

/** Print the key event to the given output stream. @version 1.0 */
inline std::ostream& operator << ( std::ostream& os, const KeyEvent& event )
{
    os << static_cast< const Event& >( event ) << " key " << event.key;
    if( event.modifiers & KM_ALT )
        os << " alt";
    if( event.modifiers & KM_CONTROL )
        os << " ctrl";
    if( event.modifiers & KM_SHIFT )
        os << " shift";
    return os;
}
}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::KeyEvent& value )
{
    byteswap( static_cast< eq::fabric::Event& >( value ));
    byteswap( value.key );
    byteswap( value.modifiers );
}
}

#endif // EQFABRIC_EVENT_H
