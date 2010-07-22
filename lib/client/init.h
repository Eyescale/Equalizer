
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_INIT_H
#define EQ_INIT_H

#include <eq/base/base.h>

/** 
 * @namespace eq
 * @brief The Equalizer client library.
 *
 * This namespace implements the application-visible API to access the Equalizer
 * server. 
 *
 * The Client represents the application instance and net::Node. A Client can
 * connect to a Server to retrieve a Config.
 * 
 * The Server, Config, Node, Window and Channel hierarchy represents the
 * rendering entities as described in the configuration file. Each process in
 * the cluster only has the local entities instantiated, that is, there is at
 * most one node per process. The Config is a net::Session, which is normally
 * used to register distributed objects based on eq::Object or eq::net::Object.
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
 * The OSWindow is used by the Window and abstracts window-system specific
 * functionality. The interface GLXWindowIF, AGLWindowIF and WGLWindowIF extend
 * the OSWindow interface by GLX, AGL and WGL-specific functionality. The
 * GLXWindow, AGLWindow and WGLWindow implement the aforementioned interfaces.
 *
 * The OSPipe, GLXPipe, AGLPipe and WGLPipe hierarchy implements a similar
 * abstraction for GPU-specific functionality.
 *
 * The GLXMessagePump, AGLMessagePump and WGLMessagePump are used by the node
 * and pipe threads to detect and dispatch system events. The GLXEventHandler,
 * AGLEventHandler and WGLEventHandler receive these events and transform them
 * into a AGLWindowEvent, GLXWindowEvent or WGLWindowEvent, respectively.
 *
 * The window events are dispatched to the corresponding OSWindow, which can
 * execute window system specific tasks. The OSWindow implementations forward
 * the generic Event to the window. The window will handle the necessary events
 * locally, and will transform the WindowEvent into a ConfigEvent, which is sent
 * to the application node using Config::sendEvent.
 *
 * The Event is a union of the possible concrete PointerEvent, KeyEvent,
 * ResizeEvent, MagellanEvent, Statistic or UserEvent and may contain a valid
 * RenderContext for a PointerEvent. The RenderContext describes the rendering
 * setup of the last Channel draw operation on the pointer position.
 *
 * During scalable rendering, a Frame is used to represent and output frame
 * during Channel::frameReadback or an input frame during
 * Channel::frameAssemble. Each Frame holds a FrameData, which is a container
 * for images and links the input with the output frames. An Image represents a
 * 2D framebuffer area, containing color and/or depth information.
 *
 * <img src="http://www.equalizergraphics.com/documents/design/images/clientUML.png">
 */
namespace eq
{
    class Config;
    class NodeFactory;

    /**
     * Initialize the Equalizer client library.
     *
     * The following command line options are recognized by this function:
     * <ul>
     *   <li>--eq-server &lt;hostname&gt; to specify an explicit server
     *         address (cf. Global::setServer())</li>
     *   <li>--eq-config &lt;filename&gt; to specify the configuration file if
     *         an application-specific server is used (cf.
     *         Global::setConfigFile())</li>
     *   <li>--eq-logfile &lt;filename&gt; to specify an output file for debug
     *         logging.</li>
     * </ul>
     *
     * Please note that further command line parameters are recognized by
     * net::Node::initLocal().
     *
     * @param argc the command line argument count.
     * @param argv the command line argument values.
     * @param nodeFactory the factory for allocating Equalizer objects.
     *
     * @return <code>true</code> if the library was successfully initialized,
     *         <code>false</code> otherwise.
     */
    EQ_EXPORT bool init( const int argc, char** argv, NodeFactory* nodeFactory);
    
    /**
     * De-initialize the Equalizer client library.
     *
     * @return <code>true</code> if the library was successfully de-initialized,
     *         <code>false</code> otherwise.
     */
    EQ_EXPORT bool exit();

    /**
     * Convenience function to retrieve a configuration.
     *
     * This function initializes a local client node, connects it to the server,
     * and retrieves a configuration. On any failure everything is correctly
     * deinitialized and 0 is returned.
     *
     * @return the pointer to a configuration, or 0 upon error.
     */
    EQ_EXPORT Config* getConfig( const int argc, char** argv );

   /** 
    * Convenience function to release a configuration.
    *
    * This function releases the configuration, disconnects the server,
    * and stops the local client node.
    */
    EQ_EXPORT void releaseConfig( Config* config );
}

#endif // EQ_INIT_H

