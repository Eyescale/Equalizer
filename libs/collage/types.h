
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_TYPES_H
#define CO_TYPES_H

#include <co/base/refPtr.h>
#include <co/base/uuid.h>

#include <deque>
#include <vector>

namespace co
{

#define CO_SEPARATOR '#'

#define EQ_INSTANCE_MAX     EQ_MAX_UINT32 //!< The biggest instance id possible
#define EQ_INSTANCE_NONE    0xfffffffdu   //!< None/NULL identifier
#define EQ_INSTANCE_INVALID 0xfffffffeu   //!< Invalid/unset instance identifier
#define EQ_INSTANCE_ALL     0xffffffffu   //!< all object instances

class Barrier;
class Command;
class CommandQueue;
class Connection;
class ConnectionDescription;
class DataIStream;
class DataOStream;
class LocalNode;
class Node;
class Object;
class ObjectDataIStream;
class Serializable;
struct ObjectVersion;

/** A unique identifier for nodes. */
typedef base::UUID NodeID;

typedef base::uint128_t uint128_t;

/** A reference pointer for Node pointers. */
typedef base::RefPtr< Node >                  NodePtr;
/** A reference pointer for LocalNode pointers. */
typedef base::RefPtr< LocalNode >             LocalNodePtr;
/** A reference pointer for Connection pointers. */
typedef base::RefPtr< Connection >            ConnectionPtr;
/** A reference pointer for ConnectionDescription pointers. */
typedef base::RefPtr< ConnectionDescription > ConnectionDescriptionPtr;

/** A vector of NodePtr's. */
typedef std::vector< NodePtr >                   Nodes;
/** A const iterator for a vector of nodes. */
typedef Nodes::const_iterator                    NodesCIter;

/** A vector of objects. */
typedef std::vector< Object* >                   Objects;
/** A const iterator for a vector of objects. */
typedef Objects::const_iterator                  ObjectsCIter;

/** A vector of Barriers. */
typedef std::vector< Barrier* >                  Barriers;

/** A vector of ConnectionPtr's. */
typedef std::vector< ConnectionPtr >             Connections;
/** A const iterator for a vector of ConnectionPtr's. */
typedef Connections::const_iterator   ConnectionsCIter;

/** A vector of ConnectionDescriptionPtr's. */
typedef std::vector< ConnectionDescriptionPtr >  ConnectionDescriptions;
/** An iterator for a vector of ConnectionDescriptionPtr's. */
typedef ConnectionDescriptions::iterator         ConnectionDescriptionsIter;
/** A const iterator for a vector of ConnectionDescriptionPtr's. */
typedef ConnectionDescriptions::const_iterator   ConnectionDescriptionsCIter;

/** @cond IGNORE */
typedef std::vector< Command* > Commands;
typedef std::deque< Command* > CommandDeque;
typedef std::vector< ObjectVersion > ObjectVersions;
typedef ObjectVersions::const_iterator ObjectVersionsCIter;
typedef std::deque< ObjectDataIStream* > ObjectDataIStreamDeque;
typedef std::vector< ObjectDataIStream* > ObjectDataIStreams;

typedef Commands::const_iterator CommandsCIter;
/** @endcond */

#ifdef EQ_USE_DEPRECATED
typedef Nodes NodeVector;
typedef Objects ObjectVector;
typedef Barriers BarrierVector;
typedef Connections ConnectionVector;
typedef ConnectionDescriptions ConnectionDescriptionVector;
typedef ObjectVersions ObjectVersionVector;
#endif
}

#endif // CO_TYPES_H
