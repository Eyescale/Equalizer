
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. 
 
   Various event-related definitions.
*/

#ifndef EQ_EVENT_H
#define EQ_EVENT_H

#include <eq/client/renderContext.h> // used as member

#include <eq/base/base.h>
#include <eq/base/log.h>

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

    struct ResizeEvent
    {
        int32_t x; // relative to screen
        int32_t y;
        int32_t w;
        int32_t h;
    };
    struct PointerEvent
    {
        int32_t x;             //<! relative to entity (window)
        int32_t y;
        int32_t dx;
        int32_t dy;
        uint32_t buttons;      //<! current state of all buttons
        uint32_t button;       //<! fired button
    };
    struct KeyEvent
    {
        uint32_t key; // KC_? for special keys, ascii code otherwise
        // TODO modifier state
    };

#   define EQ_USER_EVENT_SIZE 32
    struct UserEvent
    {
        char data[ EQ_USER_EVENT_SIZE ];
    };

    struct Event
    {
        Event();

        enum Type
        {
            EXPOSE = 0,
            RESIZE,
            POINTER_MOTION,
            POINTER_BUTTON_PRESS,
            POINTER_BUTTON_RELEASE,
            KEY_PRESS,
            KEY_RELEASE,
            WINDOW_CLOSE,
            UNKNOWN,
            USER,
            ALL // must be last
        };

        uint32_t type;

        union // event data
        {
            ResizeEvent  resize;

            PointerEvent pointerEvent;
            PointerEvent pointerMotion;
            PointerEvent pointerButtonPress;
            PointerEvent pointerButtonRelease;

            KeyEvent     keyEvent;
            KeyEvent     keyPress;
            KeyEvent     keyRelease;

            UserEvent    user;
        };
     
        RenderContext context; //<! The last rendering context at (x,y)
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream&, const Event::Type );
    EQ_EXPORT std::ostream& operator << ( std::ostream&, const Event& );
    EQ_EXPORT std::ostream& operator << ( std::ostream&, const ResizeEvent& );
    EQ_EXPORT std::ostream& operator << ( std::ostream&, const PointerEvent& );
    EQ_EXPORT std::ostream& operator << ( std::ostream&, const KeyEvent& );
}

#endif // EQ_EVENT_H

