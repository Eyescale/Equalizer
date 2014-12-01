
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

#ifndef EQFABRIC_EVENTENUMS_H
#define EQFABRIC_EVENTENUMS_H

#include <lunchbox/types.h>

namespace eq
{
namespace fabric
{
namespace eventEnums
{
/**
 * Yet another key code table to report keys in a window system independent
 * way. Ordinary keys (letters, numbers, etc) are reported using the
 * corresponding ascii code. The naming is oriented on the X11 keysym naming.
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

}
}
}

#endif // EQFABRIC_EVENTENUMS_H
