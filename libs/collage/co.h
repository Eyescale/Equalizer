
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

#ifndef CO_H
#define CO_H

/**
 * @namespace co
 * @brief Object-oriented network library.
 *
 * Collage is a cross-platform C++ library for building heterogenous,
 * distributed applications. It is used as the cluster backend for
 * Equalizer. Collage provides an abstraction of different network connections,
 * peer-to-peer messaging, discovery and synchronization as well as
 * high-performance, object-oriented, versioned data distribution. Collage is
 * designed for low-overhead multi-threaded execution which allows applications
 * to easily exploit multi-core architectures.
 *
 * On the lowest layer, Collage provides the Connection, which implements a
 * stream-oriented communication stream between two endpoints. Various protocols
 * are implemented, and can be created using Connection::create with a
 * ConnectionDescription using the corresponding ConnectionType and related
 * parameters. A ConnectionSet may be used to demultiplex input from a set of
 * connections.
 *
 * Collage offers peer-to-peer communication between processes, represented by
 * Node and LocalNode. A process typically uses one LocalNode to listen on
 * connection requests and to communicate with other nodes. A Node is a proxy
 * for a remote LocalNode. To exchange information, a Packet is send using the
 * remote node proxy. The remote local node receives the packet, wraps it in a
 * Command, and dispatches it to a CommandQueue. The queue is processed by
 * another thread, which invokes a CommandFunc. The queue and handler method are
 * pre-registered with the proper Dispatcher. The packet specializations
 * NodePacket and ObjectPacket dispatch the packet to the LocalNode or a
 * registered Object, respectively.
 *
 * The Object facilitates the data distribution for C++ classes. An object is
 * registered with a listening LocalNode, which allows the mapping of this
 * object to other instances, potentially on other nodes. Objects can be static
 * or versioned. Versioned objects can commit new versions, sync committed
 * versions and can send object packets to other instances.
 *
 * The following Figure depicts the UML class diagram of the major Collage
 * classes:
 *
 * <img src="http://www.equalizergraphics.com/documents/design/images/collage.png">

 */

#include <co/barrier.h>
#include <co/command.h>
#include <co/connection.h>
#include <co/connectionDescription.h>
#include <co/connectionType.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <co/global.h>
#include <co/init.h>
#include <co/object.h>
#include <co/base/base.h>

#endif // CO_H
