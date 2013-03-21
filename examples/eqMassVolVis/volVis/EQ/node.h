
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#ifndef MASS_VOL__NODE_H
#define MASS_VOL__NODE_H

#include <eq/eq.h>

#include <boost/shared_ptr.hpp>

namespace massVolVis
{

class RAMPool;
class VolumeInfo;
class VolumeTreeBase;
class VolumeFileInfo;


typedef boost::shared_ptr<       VolumeInfo >      VolumeInfoSPtr;
typedef boost::shared_ptr< const VolumeInfo > constVolumeInfoSPtr;

typedef boost::shared_ptr<       VolumeFileInfo >      VolumeFileInfoSPtr;
typedef boost::shared_ptr< const VolumeFileInfo > constVolumeFileInfoSPtr;

typedef boost::shared_ptr< const VolumeTreeBase > constVolumeTreeBaseSPtr;

typedef boost::shared_ptr< RAMPool > RAMPoolSPtr;

/**
 * Node is a standard EQ abstraction for a computer. It manages RAMPool, which
 * is a RAM cache that shared by all Pipes.
 */
class Node : public eq::Node
{
public:
    Node( eq::Config* parent );

    constVolumeInfoSPtr getVolumeInfo() const { return _volumeInfo; }
    constVolumeFileInfoSPtr getVolumeFileInfo() const { return _volFileInfo; }

    uint32_t getFileNameVersion() const { return _fileNameVersion; }

    constVolumeTreeBaseSPtr getVolumeTree() const { return _volTree; }

    RAMPoolSPtr getRAMPool() { return _ramPool; }

protected:
    virtual bool configInit( const eq::uint128_t& initId );
    virtual bool configExit();

    /**
     * Sets ramPool to new file if data file name changes
     */
    virtual void frameStart(  const eq::uint128_t& frameId, const uint32_t frameNumber );

private:
    void _frameStart( const eq::uint128_t& frameId, const uint32_t frameNumber );

    VolumeInfoSPtr _volumeInfo;         //!< synchronized by EQ parameters (filename, tf, etc.)
    uint32_t _fileNameVersion;          //!< Last sucessfully opened file version
    uint32_t _fileNameVersionTested;    //!< Last tested file version

    RAMPoolSPtr        _ramPool;        //!< RAM manager
    VolumeFileInfoSPtr _volFileInfo;    //!< volume parameters

    constVolumeTreeBaseSPtr _volTree;   //!< volume tree
};



}//namespace massVolVis

#endif //MASS_VOL__NODE_H