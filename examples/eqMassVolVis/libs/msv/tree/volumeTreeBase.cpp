
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "volumeTreeBase.h"

#include <msv/util/hlp.h>
#include <msv/util/md5.h>
#include <msv/util/debug.h>

#include <msv/util/fileIO.h>

#include <sstream>
#include <assert.h>
#include <cstring>
#include <bitset>

namespace massVolVis
{


void VolumeTreeBase::_replaceTree( constNodeIdVecSPtr ptr )
{
    if( ptr )
        _tree = ptr;
    else
        _tree = NodeIdVecSPtr( new NodeIdVec() );

    _updateNumberOfValidBlocks();
}


uint32_t VolumeTreeBase::getNumberOfNodes( const Vec3_ui16& srcSize, const uint32_t blockSize )
{
    if( blockSize != hlpFuncs::getPow2( blockSize ))
        throw "block dimension has to be power of two";

    if( srcSize.w < 1 || srcSize.h < 1 || srcSize.d < 1 || blockSize < 32 )
        throw "Wrong dimensions";

// get size of the cube, number of nodes and depth of the tree
    const uint32_t maxDim = std::max( std::max( srcSize.w, srcSize.h ), srcSize.d );
    uint32_t pow2size = blockSize;

    int32_t nodes     = 1;
    int nextLevelNodes = 8;
    while( pow2size < maxDim && pow2size != 0 )
    {
        pow2size <<= 1;

        if( nextLevelNodes <= 0 || nodes <= 0 )
        {
            throw "Too lagre number on nodes";
        }

        nodes += nextLevelNodes;
        nextLevelNodes <<= 3;
    }
    return nodes;
}


VolumeTreeBase::VolumeTreeBase( const Vec3_ui16& srcSize, const uint32_t blockSize )
    : _srcSize( srcSize )
    , _bs( blockSize )
    , _pow2size( blockSize )
    , _depth( 1 )
    , _nValidBlocks( 0 )
{
    if( sizeof(NodeId) != 4 )
        throw "Size of the tree's node is not 4";

    if( _bs != hlpFuncs::getPow2( _bs ))
        throw "block dimension has to be power of two";

    if( _srcSize.w < 1 || _srcSize.h < 1 || _srcSize.d < 1 || _bs < 32 )
        throw "Wrong dimensions";

// get size of the cube, number of nodes and depth of the tree
    const uint32_t maxDim = std::max( std::max( _srcSize.w, _srcSize.h ), _srcSize.d );

    int nodes          = 1;
    int nextLevelNodes = 8;
    while( _pow2size < maxDim && _pow2size != 0 )
    {
        _pow2size <<= 1;
        _depth++;

        if( nextLevelNodes <= 0 || nodes <= 0 )
        {
            throw "Too lagre number on nodes";
        }

        nodes += nextLevelNodes;
        nextLevelNodes <<= 3;
    }

    std::stringstream ss;
    ss  << "Octree info: " << std::endl
        << " max dimension:   " << _pow2size                     << std::endl
        << " depth:           " << int( _depth )                 << std::endl
        << " block size:      " << int( _bs ) << "^3"            << std::endl
        << " number of nodes: " << float( nodes ) / 1000000 << " Mio (" << nodes << ")" << std::endl
        << " tree size:       " << (nodes*sizeof(NodeId))/(1024*1024) << " MB"  << std::endl
        << " node size:       " << sizeof(NodeId) << " b" << std::endl;

    if( _depth > 8 )
    {
        throw "Number of elements is too large, basic data cube size has to be increased";
    }

    NodeIdVecSPtr tree( new NodeIdVec( nodes ) );
    _stat.resize( nodes );
    _hist.resize( nodes );
    memset( &_hist[0], 0xFF, _hist.size()*sizeof(_hist[0]) );
    memset( &((*tree)[0]), 0, tree->size()*sizeof((*tree)[0]) );

// test that tree is properly initialized with nulls
#ifndef NDEBUG
    for( size_t i = 0; i < tree->size(); ++i )
    {
        if( (*tree)[i ]     || _stat[i].value  ||
            _stat[i].cost   || _stat[i].loaded ||
            _stat[i].lastAccess || _hist[i] != 0xFFFFFFFF )
            throw "initialization is not done with empty tree";
    }
#endif

// allign real data on the lowest level to the grid structure

    if( _srcSize < Vec3_ui16( _bs )) // special case when data is actually smaller than one block
    {
        _dBox.s = Vec3_ui32( _bs/2 ) - Vec3_ui32( _srcSize )/2;
    }else
    {
        Vec3_ui32 tmpDS( _srcSize );
        tmpDS = ( tmpDS + ( _bs/2 ))/_bs;

        _dBox.s = Vec3_ui32( _pow2size/2 ) - (( tmpDS + 1 )/2 )*_bs;
    }
    _dBox.e = _dBox.s + _srcSize;

    ss << "data start|size:   " << _dBox.s << " | " << _srcSize << std::endl;

    // mark tree according to the data alligment
    _markTree( tree, 0, 1, Box_i32( Vec3_i32( _pow2size )));

    // set ids for non-empty nodes
    uint32_t id = 1;
    for( size_t i = 0; i < tree->size(); ++i )
    {
        if( (*tree)[i] )
            (*tree)[i] = id++;
        else
            _hist[i] = 0;
    }
    _nValidBlocks = id-1;
    ss << "Number of valid blocks:   " << id-1 << std::endl
       << "Done initializing" << std::endl;

    _infoString = ss.str();
    _tree = tree;
}

void VolumeTreeBase::_updateNumberOfValidBlocks()
{
    _nValidBlocks = 0;;
    for( size_t i = 0; i < _tree->size(); ++i )
    {
        if( (*_tree)[i] )
            ++_nValidBlocks;
        else
            _hist[i] = 0;
    }
}


bool VolumeTreeBase::hasVisibleData( uint32_t pos, uint32_t tfHist ) const
{
    if( pos >= _hist.size() )
        return false;

    return _hist[pos] & tfHist;
}


/**
 *  Fills _tree with data fill parameters,
 */
void VolumeTreeBase::_markTree( NodeIdVecSPtr tree, uint32_t pos, byte depth, const Box_i32 vBox )
{
    if( pos > tree->size() )
        throw "index is too hight";

// test that coordinates of blocks are passed correctly and _getAbsCoordinates works in the same way
#ifndef NDEBUG
    if( getAbsCoordinates( pos ) != vBox )
    {
        LOG_ERROR << "tree boxes are wrong: pos "
                  << pos  << "; " << vBox << " vs "
                  << getAbsCoordinates( pos ) << std::endl;

        throw "critical error";
    }
#endif

    // Check that current tree element intersects actual data.
    // If there is no intersection, don't have to mark children.
    if( !vBox.intersect( _dBox ).valid() )
        return;

    // mark tree element as occupied
    (*tree)[ pos ] = 1;

    if( depth >= _depth )
        return;

    depth++;
    pos = 1 + pos*8;

    Box_i32 quads[8];
    vBox.getQuadrants( quads );

    _markTree( tree, pos+0, depth, quads[0] );
    _markTree( tree, pos+1, depth, quads[1] );
    _markTree( tree, pos+2, depth, quads[2] );
    _markTree( tree, pos+3, depth, quads[3] );

    _markTree( tree, pos+4, depth, quads[4] );
    _markTree( tree, pos+5, depth, quads[5] );
    _markTree( tree, pos+6, depth, quads[6] );
    _markTree( tree, pos+7, depth, quads[7] );
}


byte VolumeTreeBase::getLevelFromPos( uint32_t pos )
{
    int64_t p = 0;
    byte    l = 0;
    while( p <= static_cast<int64_t>(pos) )
    {
        p = getChild( p );
        l++;
        assert( l < 255 );
    }
    return l-1;
}


void VolumeTreeBase::setHistograms( const std::vector< uint32_t > hist )
{
    uint32_t srcBytes =  hist.size()*sizeof(  hist[0] );
    uint32_t dstBytes = _hist.size()*sizeof( _hist[0] );
    memcpy( &_hist[0], &_hist[0], std::min( srcBytes, dstBytes ));
}


void VolumeTreeBase::loadHistograms( const std::string& fileName )
{
    const uint32_t bytesToRead = _hist.size()*sizeof(_hist[0]);
    util::InFile inFile;
    if( !inFile.open( fileName, std::ios::binary, true ) ||
        !inFile.read( 0, bytesToRead, &_hist[0] ))
        return;

#if 0 // Dump histogram information
    std::stringstream ss;
    for( size_t i = 0; i < getSize(); ++i )
        if( getNodeId( i ) )
            ss << std::bitset<32>(_hist[i]) << std::endl;

    std::ofstream os;
    os.open( (fileName+".txt").c_str(), std::ios_base::out );
    os << ss.str().c_str();
    os.close();
#endif
}


/**
 * Decompose 3D position from interleaved coordinates.
 * Basically returns a position of a 3D-Z-curve index.
 */
Vec3_ui32 VolumeTreeBase::getIndexPosition( uint32_t pos )
{
    Vec3_ui32 p;

    uint32_t i = 1;

    while( pos >= i )
    {
        p.x |= pos & i; pos >>= 1;
        p.y |= pos & i; pos >>= 1;
        p.z |= pos & i;

        i <<= 1;
    }

    return p;
}


namespace
{
/**
 * Returns absolute coordinates of a particular block position.
 * If "absolute" is true, then dim is treated as data size, otherwice
 * dim is a dimention of a single block.
 */
Box_i32 _getCoordinates( const uint32_t pos, uint32_t dim, const bool absolute )
{
    if( pos == 0 )
        return Box_i32( Vec3_ui32( dim ));

// Find first child of the octree level where "pos" belongs, also get current octree level.
    uint64_t firstChild  = 1;
    uint32_t octreeLevel = 1;
    do{
        firstChild = firstChild*8 + 1;
        octreeLevel++;
    }while( firstChild <= pos );

    firstChild = ( firstChild - 1 )/8;
    octreeLevel--;

    const uint32_t relativePos = pos - firstChild;

    if( absolute )
        dim >>= octreeLevel;

// get 3D pos of this child pos based on Z curve index
    const Vec3_ui32 p = VolumeTreeBase::getIndexPosition( relativePos ) * dim;

    return Box_i32( p, p+dim );
}
}// namespace


Box_i32 VolumeTreeBase::getRelativeCoordinates( const uint32_t pos ) const
{
    return _getCoordinates( pos, _bs, false );
}


Box_i32 VolumeTreeBase::getAbsCoordinates( const uint32_t pos ) const
{
    return _getCoordinates( pos, _pow2size, true );
}


Box_i32 VolumeTreeBase::getAbsDataCoordinates( const uint32_t pos ) const
{
    return getAbsCoordinates( pos ) - Vec3_i32( _dBox.s );
};


uint32_t VolumeTreeBase::getNodeId( const uint32_t pos ) const
{
    if( pos >= _tree->size())
    {
        LOG_ERROR << "Invalid position in the tree: " << pos << std::endl;
        return 0;
    }

    return (*_tree)[pos];
}


const Stat* VolumeTreeBase::getNodeStat( const uint32_t pos ) const
{
    if( pos >= _stat.size() ||
        pos >= _tree->size()   )
    {
        LOG_ERROR << "Invalid position in the tree: " << pos << std::endl;
        return 0;
    }

    if( (*_tree)[pos] == 0 )
    {
        LOG_ERROR << "Can't change stat for this tree node pos: " << pos << std::endl;
        return 0;
    }

    return &_stat[pos];
}


Stat* VolumeTreeBase::getNodeStat( const uint32_t pos )
{
    if( pos >= _stat.size() ||
        pos >= _tree->size()   )
    {
        LOG_ERROR << "Invalid position in the tree: " << pos << std::endl;
        return 0;
    }

    if( (*_tree)[pos] == 0 )
    {
        LOG_ERROR << "Can't change stat for this tree node pos: " << pos << std::endl;
        return 0;
    }

    return &_stat[pos];
}


int64_t VolumeTreeBase::getTightParentPos( const Box_i32& bb ) const
{
          Box_i32 currentBox = Box_i32( Vec3_i32( _pow2size ));
    const Box_i32 b = currentBox.intersect( bb );

    Box_i32 children[8];

    int64_t pos    =  0;
    int64_t prvPos = -1;
    while( pos < static_cast<int64_t>( _tree->size() ))
    {
        prvPos = pos;
        currentBox.getQuadrants( children );

        bool notFound = true;
        for( int i = 0; i < 8; ++i )
        {
            if( children[i].contain( b ))
            {
                currentBox = children[i];
                pos = getChild( pos ) + i;
                notFound = false;
                break;
            }
        }
        if( notFound )
            return pos;
    }
    return prvPos;
}


BoundingSphere VolumeTreeBase::getBoundingSphere( const uint32_t pos )
{
    return getAbsCoordinates( pos ).getBoundingSphere();
}



}// namespace massVolVis
















