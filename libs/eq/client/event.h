
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQ_EVENT_H
#define EQ_EVENT_H

#include <eq/client/api.h>
#include <eq/client/statistic.h>     // member
#include <eq/client/types.h>
#include <eq/fabric/renderContext.h> // member


namespace eq
{
    /**
     * Yet another key code table to report keys in a window system
     * independent way. Ordinary keys (letters, numbers, etc) are reported
     * using the corresponding ascii code. The naming is oriented on the X11
     * keysym naming.
     */
    enum KeyCode
    {
        KC_ESCAPE = 256,
        KC_BACKSPACE,
        KC_RETURN,
        KC_TAB,
        KC_HOME,
        KC_LEFT,
        KC_UP,
        KC_RIGHT,
        KC_DOWN,
        KC_PAGE_UP,
        KC_PAGE_DOWN,
        KC_END,
        KC_F1,
        KC_F2,
        KC_F3,
        KC_F4,
        KC_F5,
        KC_F6,
        KC_F7,
        KC_F8,
        KC_F9,
        KC_F10,
        KC_F11,
        KC_F12,
        KC_F13,
        KC_F14,
        KC_F15,
        KC_F16,
        KC_F17,
        KC_F18,
        KC_F19,
        KC_F20,
        KC_F21,
        KC_F22,
        KC_F23,
        KC_F24,
        KC_SHIFT_L,
        KC_SHIFT_R,
        KC_CONTROL_L,
        KC_CONTROL_R,
        KC_ALT_L,
        KC_ALT_R,
        KC_VOID = 0xFFFFFF /* == XK_VoidSymbol */
    };

    /**
     * Mouse pointer button definition. The enums are defined as masks, so that
     * the state of all buttons can be OR'd using the same enum.
     * @version 1.0
     */
    enum PointerButton
    {
        PTR_BUTTON_NONE = LB_BIT_NONE,
        PTR_BUTTON1     = LB_BIT1,
        PTR_BUTTON2     = LB_BIT2,
        PTR_BUTTON3     = LB_BIT3,
        PTR_BUTTON4     = LB_BIT4,
        PTR_BUTTON5     = LB_BIT5,
        PTR_BUTTON6     = LB_BIT6,
        PTR_BUTTON7     = LB_BIT7
    };

    /**
     * Event for a size or position change on a Window, Channel or View.
     * @version 1.0
     */
    struct ResizeEvent
    {
        int32_t x; //!< new X position, relative to parent
        int32_t y; //!< new Y position, relative to parent
        int32_t w; //!< new width
        int32_t h; //!< new height
        float dw;  //!< view only: new width relative to initial width
        float dh;  //!< view only: new height relative to initial height
    };

    /** Event for a pointer (mouse) motion or click. @version 1.0 */
    struct PointerEvent
    {
        int32_t x;             //!< X position relative to entity
        int32_t y;             //!< Y position relative to entity (0 is on top)
        int32_t dx;            //!< X position change since last event
        int32_t dy;            //!< Y position change since last event
        uint32_t buttons;      //!< current state of all buttons
        uint32_t button;       //!< fired button
        int32_t xAxis;         //!< x wheel rotation
        int32_t yAxis;         //!< y wheel rotation
    };

    /** Event for a key press or release. @version 1.0 */
    struct KeyEvent
    {
        uint32_t key; //!<  KeyCode for special keys, ascii code otherwise
        // TODO modifier state
    };

    /**
     * Event for a SpaceMouse movement or click.
     * @warning experimental - may not be supported in the future.
     */
    struct MagellanEvent
    {
        uint32_t button;       //!< fired button
        uint32_t buttons;      //!< current state of all buttons
        int16_t xAxis;         //!< X translation
        int16_t yAxis;         //!< Y translation
        int16_t zAxis;         //!< Z translation
        int16_t xRotation;     //!< X rotation
        int16_t yRotation;     //!< Y rotation
        int16_t zRotation;     //!< Z rotation
    };

#   define EQ_USER_EVENT_SIZE 128
    /**
     * User-defined event.
     *
     * See the eqPixelBench example on how to use user-defined events.
     * @version 1.0
     */
    struct UserEvent
    {
        char data[ EQ_USER_EVENT_SIZE ]; //!< Application-specific data
    };

    /**
     * Event structure to report window system and other events.
     *
     * Depending on the Event::Type, the corresponding specific event data is
     * filled into the anonymous union. The originator typically contains the
     * co::Object identifier of the entity emitting the event. The rendering
     * context is only set for pointer events.
     * @version 1.0
     */
    struct Event
    {
        /** Construct a new event. */
        EQ_API Event();

        /** The type of the event. */
        enum Type // Also update string table in event.cpp
        {
            WINDOW_EXPOSE = 0,    //!< A window is dirty
            WINDOW_RESIZE,        //!< Window resize data in resize
            WINDOW_CLOSE,         //!< A window has been closed
            WINDOW_HIDE,          //!< A window is hidden
            WINDOW_SHOW,          //!< A window is shown
            WINDOW_SCREENSAVER,   //!< A window screensaver request (Win32 only)
#ifdef EQ_USE_DEPRECATED
            POINTER_MOTION,       //!< Pointer movement data in pointerMotion
            /** Pointer button press data in pointerButtonPress */
            POINTER_BUTTON_PRESS,
            /** Pointer button release data in pointerButtonRelease */
            POINTER_BUTTON_RELEASE,
            POINTER_WHEEL,        //!< Mouse wheel data in wheel
            //!< Channel pointer movement data in pointerMotion
            CHANNEL_POINTER_MOTION = POINTER_MOTION,
            /** Channel pointer button press data in pointerButtonPress */
            CHANNEL_POINTER_BUTTON_PRESS = POINTER_BUTTON_PRESS,
            /** Channel pointer button release data in pointerButtonRelease */
            CHANNEL_POINTER_BUTTON_RELEASE = POINTER_BUTTON_RELEASE,
            //!< Window pointer Mouse wheel data in wheel
            WINDOW_POINTER_WHEEL = POINTER_WHEEL,
#else
           //!< Channel pointer movement data in pointerMotion
            CHANNEL_POINTER_MOTION,
            /** Channel pointer button press data in pointerButtonPress */
            CHANNEL_POINTER_BUTTON_PRESS,
            /** Channel pointer button release data in pointerButtonRelease */
            CHANNEL_POINTER_BUTTON_RELEASE,
            //!< Window pointer Mouse wheel data in wheel
            WINDOW_POINTER_WHEEL,
#endif
           //!< Window pointer movement data in pointerMotion
            WINDOW_POINTER_MOTION,
            /** Window pointer button press data in pointerButtonPress */
            WINDOW_POINTER_BUTTON_PRESS,
            /** Window pointer button release data in pointerButtonRelease */
            WINDOW_POINTER_BUTTON_RELEASE,
            /** Window pointer grabbed by system window */
            WINDOW_POINTER_GRAB,
            /** Window pointer to be released by system window */
            WINDOW_POINTER_UNGRAB,

            KEY_PRESS,            //!< Key press data in keyPress
            KEY_RELEASE,          //!< Key release data in keyRelease
            CHANNEL_RESIZE,       //!< Channel resize data in resize
            STATISTIC,            //!< Statistic event in statistic
            VIEW_RESIZE,          //!< View resize data in resize
            EXIT,                 //!< Exit request due to runtime error
            MAGELLAN_AXIS,        //!< SpaceMouse movement data in magellan
            MAGELLAN_BUTTON,      //!< SpaceMouse button data in magellan
            UNKNOWN,              //!< Event type not known by the event handler
            /** User-defined events have to be of this type or higher */
            USER = UNKNOWN + 5, // some buffer for binary-compatible patches
            ALL // must be last
        };


        uint32_t type;           //!< The event type

        // keep before 'uint128_t originator' for alignment
        uint32_t serial;       //!< server-unique originator serial number

        /** The config time when the event was created. @version 1.1.1 */
        int64_t time;

        /** The identifier of the entity emitting the event. */
        uint128_t originator;

        /** Data for the event corresponding to the event type. */
        union
        {
            ResizeEvent   resize;             //!< Resize event data
            ResizeEvent   show;               //!< Window show event data
            ResizeEvent   hide;               //!< Window hide event data

            PointerEvent  pointer;            //!< Pointer event data
            PointerEvent  pointerMotion;      //!< Pointer motion data
            PointerEvent  pointerButtonPress; //!< Mouse button press data
            PointerEvent  pointerButtonRelease; //!< Mouse button release data
            PointerEvent  pointerWheel;       //!< Mouse wheel motion data

            KeyEvent      key;                //!< Key event data
            KeyEvent      keyPress;           //!< Key press event data
            KeyEvent      keyRelease;         //!< Key release event data

            Statistic     statistic;          //!< Statistic event
            MagellanEvent magellan;           //!< SpaceMouse data

            UserEvent     user;               //!< User-defined event data
        };

        /** The last rendering context for the pointer position. */
        RenderContext context;
    };

    /** Print the event to the given output stream. @version 1.0 */
    EQ_API std::ostream& operator << ( std::ostream&, const Event& );
    /** Print the event type to the given output stream. @version 1.0 */
    EQ_API std::ostream& operator << ( std::ostream&, const Event::Type& );
    /** Print the resize event to the given output stream. @version 1.0 */
    EQ_API std::ostream& operator << ( std::ostream&, const ResizeEvent& );
    /** Print the pointer event to the given output stream. @version 1.0 */
    EQ_API std::ostream& operator << ( std::ostream&, const PointerEvent& );
    /** Print the key event to the given output stream. @version 1.0 */
    EQ_API std::ostream& operator << ( std::ostream&, const KeyEvent& );
    /** Print the space mouse event to the given output stream. @version 1.0 */
    EQ_API std::ostream& operator << ( std::ostream&, const MagellanEvent& );
}

namespace lunchbox
{
template<> inline void byteswap( eq::Event& value )
{
    byteswap( value.type );
    byteswap( value.serial );
    byteswap( value.time );
    byteswap( value.originator );
    // #145 Todo byteswap union
    //byteswap( value.union );
    byteswap( value.context );
}
}

#endif // EQ_EVENT_H
