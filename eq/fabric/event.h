
/* Copyright (c) 2006-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Cedric Stalder <cedric.stalder@gmail.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQFABRIC_EVENT_H
#define EQFABRIC_EVENT_H

#include <eq/fabric/api.h>
#include <eq/fabric/types.h>
#include <eq/fabric/renderContext.h> // member
#include <eq/fabric/statistic.h>     // member


namespace eq
{
namespace fabric
{

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
    int32_t x;        //!< X position relative to entity
    int32_t y;        //!< Y position relative to entity (0 is on top)
    int32_t dx;       //!< X position change since last event
    int32_t dy;       //!< Y position change since last event
    uint32_t buttons; //!< current state of all buttons
    uint32_t button;  //!< fired button
    float xAxis;      //!< x wheel rotation in clicks
    float yAxis;      //!< y wheel rotation in clicks
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
 * Depending on the Event::Type, the corresponding specific event data is filled
 * into the anonymous union. The originator typically contains the co::Object
 * identifier of the entity emitting the event. The rendering context is only
 * set for pointer events.
 * @version 1.0
 */
struct Event
{
    /** Construct a new event. */
    EQFABRIC_API Event();

    /** The type of the event. */
    enum Type // Also update string table in event.cpp
    {
        WINDOW_EXPOSE = 0,    //!< A window is dirty
        WINDOW_RESIZE,        //!< Window resize data in resize
        WINDOW_CLOSE,         //!< A window has been closed
        WINDOW_HIDE,          //!< A window is hidden
        WINDOW_SHOW,          //!< A window is shown
        WINDOW_SCREENSAVER,   //!< A window screensaver request (Win32 only)
        //!< Channel pointer movement data in pointerMotion
        CHANNEL_POINTER_MOTION,
        /** Channel pointer button press data in pointerButtonPress */
        CHANNEL_POINTER_BUTTON_PRESS,
        /** Channel pointer button release data in pointerButtonRelease */
        CHANNEL_POINTER_BUTTON_RELEASE,
        //!< Channel pointer Mouse wheel data in wheel
        CHANNEL_POINTER_WHEEL,
        //!< Window pointer Mouse wheel data in wheel
        WINDOW_POINTER_WHEEL,
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
        NODE_TIMEOUT,         //!< Node has timed out

        /**
         * Observer moved (head tracking update). Contains observer originator
         * identifier and 4x4 float tracking matrix.
         * @version 1.5.2
         */
        OBSERVER_MOTION,

        /**
         * Config error event. Contains the originator id, the error code and
         * 0-n Strings with additional information.
         * @version 1.7.1
         */
        CONFIG_ERROR,
        NODE_ERROR, //!< Node error event. @sa CONFIG_ERROR
        PIPE_ERROR, //!< Pipe error event. @sa CONFIG_ERROR
        WINDOW_ERROR, //!< Window error event. @sa CONFIG_ERROR
        CHANNEL_ERROR, //!< Channel error event. @sa CONFIG_ERROR

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
EQFABRIC_API std::ostream& operator << ( std::ostream&, const Event& );
/** Print the event type to the given output stream. @version 1.0 */
EQFABRIC_API std::ostream& operator << ( std::ostream&, const Event::Type& );
/** Print the resize event to the given output stream. @version 1.0 */
EQFABRIC_API std::ostream& operator << ( std::ostream&, const ResizeEvent& );
/** Print the pointer event to the given output stream. @version 1.0 */
EQFABRIC_API std::ostream& operator << ( std::ostream&, const PointerEvent& );
/** Print the key event to the given output stream. @version 1.0 */
EQFABRIC_API std::ostream& operator << ( std::ostream&, const KeyEvent& );
/** Print the space mouse event to the given output stream. @version 1.0 */
EQFABRIC_API std::ostream& operator << ( std::ostream&, const MagellanEvent& );
}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::Event& value )
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

#endif // EQFABRIC_EVENT_H
