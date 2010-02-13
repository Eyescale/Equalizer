
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/client/renderContext.h> // member
#include <eq/client/statistic.h>     // member

#include <eq/base/base.h>

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
     */
    enum PointerButton
    {
        PTR_BUTTON_NONE = EQ_BIT_NONE,
        PTR_BUTTON1     = EQ_BIT1,
        PTR_BUTTON2     = EQ_BIT2,
        PTR_BUTTON3     = EQ_BIT3,
        PTR_BUTTON4     = EQ_BIT4,
        PTR_BUTTON5     = EQ_BIT5
    };

    /** Event for a size or position change on a Window, Channel or View. */
    struct ResizeEvent
    {
        int32_t x; //!< new X position, relative to parent
        int32_t y; //!< new Y position, relative to parent
        int32_t w; //!< new width
        int32_t h; //!< new height
        float dw;  //!< view only: new width relative to initial width
        float dh;  //!< view only: new height relative to initial height
    };

    /** Event for a pointer (mouse) motion or click. */
    struct PointerEvent
    {
        int32_t x;             //!< X position relative to entity
        int32_t y;             //!< Y position relative to entity
        int32_t dx;            //!< X position change since last event
        int32_t dy;            //!< Y position change since last event
        uint32_t buttons;      //!< current state of all buttons
        uint32_t button;       //!< fired button
    };

    /** Event for a key press or release. */
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

#   define EQ_USER_EVENT_SIZE 64
    /**
     * User-defined event.
     *
     * See the eqPixelBench example on how to use user-defined events.
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
     * net::Object identifier of the entity emitting the event. The rendering
     * context is only set for pointer events.
     */
    struct Event
    {
        /** Construct a new event. */
        EQ_EXPORT Event();

        /** The type of the event. */
        enum Type // Also update string table in event.cpp
        {
            WINDOW_EXPOSE = 0,    //!< A window is dirty
            WINDOW_RESIZE,        //!< Window resize data in resize
            WINDOW_CLOSE,         //!< A window has been closed 
            WINDOW_HIDE,          //!< A window is hidden
            WINDOW_SHOW,          //!< A window is shown
            WINDOW_SCREENSAVER,   //!< A window screensaver request (Win32 only)
            POINTER_MOTION,       //!< Pointer movement data in pointerMotion
            /** Pointer button press data in pointerButtonPress */
            POINTER_BUTTON_PRESS,
            /** Pointer button release data in pointerButtonRelease */
            POINTER_BUTTON_RELEASE,
            KEY_PRESS,            //!< Key press data in keyPress
            KEY_RELEASE,          //!< Key release data in keyRelease
            CHANNEL_RESIZE,       //!< Channel resize data in resize
            STATISTIC,            //!< Statistic event in statistic
            VIEW_RESIZE,          //!< View resize data in resize
            EXIT,                 //!< Exit request due to runtime error
            MAGELLAN_AXIS,        //!< SpaceMouse movement data in magellan
            MAGELLAN_BUTTON,      //!< SpaceMouse button data in magellan
            UNKNOWN,              //!< Event type not known by the event handler
            FILL1,  // some buffer for binary-compatible patches
            FILL2,
            FILL3,
            FILL4,
            FILL5,
            /** User-defined events have to be of this type or higher */
            USER,
            ALL // must be last
        };

        /** The event type */
        uint32_t type;

        /** The identifier of the entity emitting the event. */
        uint32_t originator;

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

    EQ_EXPORT std::ostream& operator << ( std::ostream&, const Event& );
    EQ_EXPORT std::ostream& operator << ( std::ostream&, const Event::Type& );
    EQ_EXPORT std::ostream& operator << ( std::ostream&, const ResizeEvent& );
    EQ_EXPORT std::ostream& operator << ( std::ostream&, const PointerEvent& );
    EQ_EXPORT std::ostream& operator << ( std::ostream&, const KeyEvent& );
    EQ_EXPORT std::ostream& operator << ( std::ostream&, const Statistic& );
    EQ_EXPORT std::ostream& operator << ( std::ostream&, const MagellanEvent& );
}

#endif // EQ_EVENT_H

