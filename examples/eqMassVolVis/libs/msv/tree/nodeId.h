
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */

#ifndef MASS_VOL__NODE_ID_H
#define MASS_VOL__NODE_ID_H

#include <stdint.h>
#include <vector>
#include <msv/types/types.h>

typedef uint32_t NodeId;


struct NodeIdPos
{
    NodeIdPos() : id(0), treePos(0) {}

    NodeIdPos( NodeId id_, uint32_t treePos_ )
        : id(  id_  )
        , treePos( treePos_ ){}

    NodeId   id;
    uint32_t treePos;

    bool operator == ( const NodeIdPos& other ) const
    {
        return id      == other.id      &&
               treePos == other.treePos;
    }
};

typedef std::vector<NodeId>     NodeIdVec;
typedef std::vector<NodeIdPos>  NodeIdPosVec;

#endif // MASS_VOL__NODE_ID_H
