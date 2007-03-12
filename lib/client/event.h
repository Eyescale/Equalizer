/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. 
 
   Various event-related definitions.
*/

#ifndef EQ_EVENT_H
#define EQ_EVENT_H

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
        PTR_BUTTON_NONE = 0,
        PTR_BUTTON1     = 0x01,
        PTR_BUTTON2     = 0x02,
        PTR_BUTTON3     = 0x04,
        PTR_BUTTON4     = 0x08,
        PTR_BUTTON5     = 0x10
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
        int32_t x; // relative to entity (window)
        int32_t y;
        int32_t dx;
        int32_t dy;
        uint32_t buttons; // current state of all buttons
        uint32_t button;  // fired button
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
}

#endif // EQ_EVENT_H

