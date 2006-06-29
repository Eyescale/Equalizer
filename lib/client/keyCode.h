/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_KEYCODE_H
#define EQ_KEYCODE_H

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
        KC_SHIFT_L,
        KC_SHIFT_R,
        KC_CONTROL_L,
        KC_CONTROL_R,
        KC_ALT_L,
        KC_ALT_R,
        KC_VOID = 0xFFFFFF /* == XK_VoidSymbol */
    };
}

#endif // EQ_KEYCODE_H

