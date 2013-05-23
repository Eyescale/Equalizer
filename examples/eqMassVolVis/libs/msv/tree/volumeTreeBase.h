
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__VOLUME_TREE_BASE_H
#define MASS_VOL__VOLUME_TREE_BASE_H

#include "nodeId.h"

#include <msv/types/box.h>
#include <msv/types/vec3.h>
#include <msv/types/types.h>

#include <vector>

#include <boost/shared_ptr.hpp>

namespace massVolVis
{


struct Stat
{
    Stat() : value(0), cost(0), loaded(0), lastAccess(0) {}

    byte value;       // (importance), 0 means no data
    byte cost;        // rendering cost
    byte loaded;      // true if memory block was loaded
    byte lastAccess;  // how long ago it was accessed
};


typedef boost::shared_ptr<       NodeIdVec >      NodeIdVecSPtr;
typedef boost::shared_ptr< const NodeIdVec > constNodeIdVecSPtr;


/**
 *  Tree manager
 */
class VolumeTreeBase
{
private:
    VolumeTreeBase(){}

public:
    explicit VolumeTreeBase( const Vec3_ui16& srcSize, const uint32_t blockSize = 128 );

    virtual ~VolumeTreeBase(){}

    /* Empty nodes have 0 nodeId*/
    virtual uint32_t getNodeId( const uint32_t pos ) const;

    //TODO: replace with getBlockDim()
    int getBlockSize() const { return _bs; }

    /** Using _bs returns relative coordinates of a particular block position.
      * Without parameters returns coordinates of the whole data.
      */
    Box_i32 getRelativeCoordinates( const uint32_t pos = 0 ) const;

    /** Using _pow2size returns absolute coordinates of a particular block position.
      * Without parameters returns coordinates of the whole data.
      */
    Box_i32 getAbsCoordinates( const uint32_t pos = 0 ) const;

    /** Returns absolute coordinates of a data within a particular block position.
      * Without parameters returns coordinates of the whole data.
      */
    Box_i32 getAbsDataCoordinates( const uint32_t pos = 0 ) const;


    const Box_i32& getDataCoordinates() const { return _dBox; }

    /** Returns position of a node that includes desired bb, but there is no separate child
        of that block that also includes it */
    int64_t getTightParentPos( const Box_i32& bb ) const;

    static inline int64_t getParent( const uint32_t pos ) { return pos == 0 ? -1 : (pos-1)/8; };
    static inline int64_t getChild(  const uint32_t pos ) { return pos*8+1; };

    bool hasChildren( const uint32_t pos ) const { return getChild( pos ) + 7 < getSize(); };

    uint32_t getSize() const { return _tree ? _tree->size() : 0; }

    byte getDepth() const { return _depth; }

    int  getMaxBBSize() const { return _pow2size; }

    /* Returns number of valid blocks; since numbering starts from 1
       the value is also the last valid node Id (not to mix with
       max index, which can be obtained as getSize())
     */
    uint32_t getNumberOfValidBlocks() const { return _nValidBlocks; }

    const Stat * getNodeStat( const uint32_t pos ) const;
          Stat * getNodeStat( const uint32_t pos );

    BoundingSphere getBoundingSphere( const uint32_t pos );

    const std::string& getInfoString() const { return _infoString; }

    static byte getLevelFromPos( uint32_t pos );

    static Vec3_ui32 getIndexPosition( uint32_t pos );

    void setHistograms( const std::vector< uint32_t > hist );
    void loadHistograms( const std::string& fileName );

    bool hasVisibleData( uint32_t pos, uint32_t tfHist ) const;

    static uint32_t getNumberOfNodes( const Vec3_ui16& srcSize, uint32_t blockSize );

protected:
    void _updateNumberOfValidBlocks();

    void _replaceTree( constNodeIdVecSPtr ptr );
    void _markTree( NodeIdVecSPtr tree, uint32_t pos, byte depth, const Box_i32 vBox );

    Vec3_ui16   _srcSize;   // original size of the data
    uint32_t    _bs;        // block size (we have cubes, also this is constant for all blocks)

    Box_i32     _dBox;      // data box (relative to the BB octree)
    uint32_t    _pow2size;  // final BB octree size
    byte        _depth;     // octree depth

    uint32_t    _nValidBlocks; // number of valid blocs

    constNodeIdVecSPtr      _tree; // tree itself (doesn't change over time)
    std::vector< Stat     > _stat; // tree statistics (runtime data)
    std::vector< uint32_t > _hist; // histograms

    std::string _infoString;

}; // class VolumeTreeBase

} // namespace massVolVis


#endif // MASS_VOL__VOLUME_TREE_BASE_H



