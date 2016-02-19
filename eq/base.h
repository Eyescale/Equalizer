
/* Copyright (c) 2010-2016, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQCLIENT_H
#define EQCLIENT_H

/**
 * @namespace eq
 * @brief The Equalizer client library.
 *
 * This namespace implements the application-visible API to access the Equalizer
 * server.
 *
 * The Client represents the application instance and co::Node. A Client can
 * connect to a Server to retrieve a Config.
 *
 * The Server, Config, Node, Window and Channel hierarchy represents the
 * rendering entities as described in the configuration file. Each process in
 * the cluster only has the local entities instantiated, that is, there is at
 * most one node per process. The Config is a co::Session, which is normally
 * used to register distributed objects based on eq::Object or co::Object.
 *
 * The Canvas, Segment, Layout, View and Observer hierarchy represents the
 * physical projection system, logical layout and tracked users. Applications
 * which want to be multi-view capable typically extend the View to attach
 * application-specific data, e.g., a camera position or model.
 *
 * A ConfigVisitor, NodeVisitor, PipeVisitor, WindowVisitor, ChannelVisitor,
 * CanvasVisitor, SegmentVisitor, LayoutVisitor, ViewVisitor or ObserverVisitor
 * may be used to traverse the corresponding entity and to execute methods for
 * each child of the hierarchy.
 *
 * The SystemWindow is used by the Window and abstracts window-system specific
 * functionality. The GLWindow implements generic OpenGL functionality, and is
 * in turn sub-classed by the interfaces GLXWindowIF, AGLWindowIF and
 * WGLWindowIF, which extend the GLWindow by GLX, AGL and WGL-specific
 * functionality. The GLXWindow, AGLWindow and WGLWindow implement the
 * aforementioned interfaces.
 *
 * The SystemPipe, GLXPipe, AGLPipe and WGLPipe hierarchy implements a similar
 * abstraction for GPU-specific functionality.
 *
 * The GLXMessagePump, AGLMessagePump and WGLMessagePump are used by the node
 * and pipe threads to detect and dispatch system events. The GLXEventHandler,
 * AGLEventHandler and WGLEventHandler receive these events and transform them
 * into a AGLWindowEvent, GLXWindowEvent or WGLWindowEvent, respectively.
 *
 * The window events are dispatched to the corresponding SystemWindow, which can
 * execute window system specific tasks. The SystemWindow implementations
 * forward the generic Event to the window. The window will handle the necessary
 * events locally, and will transform the WindowEvent into a ConfigEvent, which
 * is sent to the application node using Config::sendEvent().
 *
 * The Event is a union of the possible concrete PointerEvent, KeyEvent,
 * ResizeEvent, MagellanEvent, Statistic or UserEvent and may contain a valid
 * RenderContext for a PointerEvent. The RenderContext describes the rendering
 * setup of the last Channel draw operation on the pointer position.
 *
 * During scalable rendering, a Frame is used to represent an output frame
 * during Channel::frameReadback or an input frame during
 * Channel::frameAssemble. Each Frame holds a FrameData, which is a container
 * for images and links the input with the output frames. An Image represents a
 * 2D framebuffer area, containing color and/or depth information.
 *
 * <img src="http://www.equalizergraphics.com/documents/design/images/clientUML.png">
 */

#include <eq/canvas.h>
#include <eq/channelStatistics.h>
#include <eq/channel.h>
#include <eq/client.h>
#include <eq/compositor.h>
#include <eq/config.h>
#include <eq/eventICommand.h>
#include <eq/error.h>
#include <eq/exception.h>
#include <eq/frame.h>
#include <eq/frameData.h>
#include <eq/global.h>
#include <eq/glException.h>
#include <eq/image.h>
#include <eq/imageOp.h>
#include <eq/init.h>
#include <eq/layout.h>
#include <eq/log.h>
#include <eq/messagePump.h>
#include <eq/node.h>
#include <eq/nodeFactory.h>
#include <eq/observer.h>
#include <eq/pipe.h>
#include <eq/pixelData.h>
#include <eq/server.h>
#include <eq/segment.h>
#include <eq/systemWindow.h>
#include <eq/types.h>
#include <eq/version.h>
#include <eq/view.h>
#include <eq/window.h>
#include <eq/windowSystem.h>

#ifndef EQ_2_0_API
#  include <eq/configEvent.h>
#endif

#ifdef EQ_SYSTEM_INCLUDES
#  include <eq/gl.h>
#  include <eq/system.h>
#endif

#endif // EQCLIENT_H
