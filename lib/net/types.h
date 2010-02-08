
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

#ifndef EQNET_TYPES_H
#define EQNET_TYPES_H

#include <eq/base/hash.h>
#include <eq/base/refPtr.h>
#include <eq/base/uuid.h>

#include <deque>
#include <list>
#include <vector>

namespace eq
{
namespace net
{

class Node;
class Session;
class Object;
class Barrier;
class Command;
class Connection;
class ConnectionDescription;
class ObjectInstanceDataIStream;

/** A unique identifier for nodes. */
typedef base::UUID NodeID;

/** A unique identifier for sessions. */
typedef base::UUID SessionID;

/** A reference pointer for Node pointers. */
typedef base::RefPtr< Node >                  NodePtr;
/** A reference pointer for Connection pointers. */
typedef base::RefPtr< Connection >            ConnectionPtr;
/** A reference pointer for ConnectionDescription pointers. */
typedef base::RefPtr< ConnectionDescription > ConnectionDescriptionPtr;

/** A vector of NodePtr's. */
typedef std::vector< NodePtr >                   NodeVector;
/** A vector of Objects. */
typedef std::vector< Object* >                   ObjectVector;
/** A vector of Barriers. */
typedef std::vector< Barrier* >                  BarrierVector;
/** A vector of ConnectionPtr's. */
typedef std::vector< ConnectionPtr >             ConnectionVector;
/** A vector of ConnectionDescriptionPtr's. */
typedef std::vector< ConnectionDescriptionPtr >  ConnectionDescriptionVector;

/** @cond IGNORE */
typedef std::vector< Command* > CommandVector;
typedef std::deque< Command* > CommandDeque;
typedef stde::hash_map< uint32_t, ObjectVector > ObjectVectorHash;
typedef std::list< Command* >   CommandList;
typedef std::deque< ObjectInstanceDataIStream* > InstanceDataDeque;
typedef std::vector< ObjectInstanceDataIStream* > InstanceDataVector;
/** @endcond */
}
}

#endif // EQNET_TYPES_H
