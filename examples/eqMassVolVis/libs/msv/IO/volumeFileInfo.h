
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */

#ifndef MASS_VOL__OCTREE_INFO_H
#define MASS_VOL__OCTREE_INFO_H

#include <msv/types/box.h>
#include <msv/types/vec2.h>
#include <msv/types/vec3.h>
#include <msv/types/types.h>


#include <msv/types/bits.h>

#include <vector>

#include <boost/shared_ptr.hpp>


namespace massVolVis
{

class VolumeTreeBase;
class DataHDDIO;

typedef boost::shared_ptr< DataHDDIO > DataHDDIOSPtr;

/**
 *  Information about stored data
 */
class VolumeFileInfo
{
public:
    enum DataType
    {
        VERSION      = Bits::B_1,
        SOURCE_DIMS  = Bits::B_2,
        BLOCK_DIM    = Bits::B_3,
        BORDER_DIM   = Bits::B_4,
        BYTES        = Bits::B_5,
        COMPRESSION  = Bits::B_6,
        DATA_FILE    = Bits::B_7,
        TF_FILE      = Bits::B_8,
    };

    enum CompressionType
    {
        NONE                       = 0,
    };

    VolumeFileInfo();
    explicit VolumeFileInfo( const VolumeFileInfo& fileInfo );
    virtual ~VolumeFileInfo();

    virtual void reset();
    virtual void setDefaults();

    virtual void setVersion(      const int             version  ) { _version      = version;  _setAttribute( VERSION     ); }
    virtual void setSourceDims(   const Vec3_ui16&      dims     ) { _srcDims      = dims;     _setAttribute( SOURCE_DIMS ); }
    virtual void setBlockDim(     const uint32_t        blockDim ) { _blockDim     = blockDim; _setAttribute( BLOCK_DIM   ); }
    virtual void setBorderDim(    const byte            border   ) { _border       = border;   _setAttribute( BORDER_DIM  ); }
    virtual void setBytesNum(     const byte            bytes    ) { _bytes        = bytes;    _setAttribute( BYTES       ); }
    virtual void setCompression(  const CompressionType cmpr     ) { _compression  = cmpr;     _setAttribute( COMPRESSION ); }
    virtual void setDataFileName( const std::string&    fileName );
    virtual void setTFFileName(   const std::string&    fileName ) { _tfFileName   = fileName; _setAttribute( TF_FILE     ); }

    virtual uint32_t getBlockSize_() const;

    /**
     * @param [out] hddIO Data IO, based on parameters and compression
     */
    virtual DataHDDIOSPtr createDataHDDIO( bool initTree = true );

    int                 getVersion()      const { return _version;      }
    const Vec3_ui16&    getSourceDims()   const { return _srcDims;      }
    uint32_t            getBlockDim()     const { return _blockDim;     }
    byte                getBorderDim()    const { return _border;       }
    byte                getBytesNum()     const { return _bytes;        }
    CompressionType     getCompression()  const { return _compression;  }
    const std::string&  getDataFileName() const { return _dataFileName; }
    const std::string&  getDataFileDir()  const { return _dataFileDir;  }
    const std::string&  getTFFileName()   const { return _tfFileName;   }
          std::string   getHistorgamFileName() const { return getDataFileDir() + "/hist.raw"; }

    uint32_t            getBlockAndBordersDim() const { return _blockDim + _border*2; }

    bool load( const std::string& file );        //!< Load data info from a file
    bool save( const std::string& file )  const; //!< Save data info to a file

    bool isAttributeSet( const DataType dataType ) const;

    friend std::ostream& operator<<( std::ostream& out, const VolumeFileInfo& info );

private:
    void _updateBlockSize();
    void _setAttribute( const DataType dataType );

    uint32_t        _attributes; //!< show if some attributes were loaded

    int             _version;       //!< config file version
    Vec3_ui16       _srcDims;       //!< original dimensions of the data
    uint32_t        _blockDim;      //!< block dimention (we have cubes, also this is constant for all blocks)
    byte            _border;        //!< border size (borders are symmetrical for all dimensions)
    byte            _bytes;         //!< number of bytes per value (usually 1 or 2, i.e. 8 or 16 bit data)
    CompressionType _compression;   //!< type of compression used for the data
    std::string     _dataFileName;  //!< for raw data - name of the file, for octree - name of the folder
    std::string     _dataFileDir;   //!< name of the folder
    std::string     _tfFileName;    //!< Transfer Function file name

};

}// namespace massVolVis

#endif //MASS_VOL__OCTREE_INFO_H