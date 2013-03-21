//std12

#ifndef MASS_VOL__COMPRESSION_H
#define MASS_VOL__COMPRESSION_H

#include <boost/shared_ptr.hpp>
#include <GL/glew.h>
#include <msv/types/types.h>

namespace eq { namespace util { class PixelBufferObject; }}

namespace massVolVis
{

class DecompressorBase;
class DataHDDIO;
class GPULoadRequest;
class RAMDataElement;
class VolumeTreeBase;

typedef boost::shared_ptr< DecompressorBase > DecompressorSPtr;
typedef boost::shared_ptr<const DataHDDIO> constDataHDDIOSPtr;
typedef boost::shared_ptr< eq::util::PixelBufferObject > PboSPtr;
typedef boost::shared_ptr< const VolumeTreeBase > constVolumeTreeBaseSPtr;


class Decompressors
{
public:
    static DecompressorSPtr select( constDataHDDIOSPtr      dataIOSPtr,
                                    constVolumeTreeBaseSPtr volumeTreeSPtr,
                                    PboSPtr                 pboSPtr,
                                    uint32_t                capacity );
};


class DecompressorBase
{
public:
    DecompressorBase( constDataHDDIOSPtr dataIOSPtr, PboSPtr pboSPtr );
    virtual ~DecompressorBase(){}

    virtual bool load( const RAMDataElement* dataEl, GPULoadRequest& request ) = 0;

    virtual bool supportsFastReloading() const { return false; }

    bool isIntialized() const { return _initialized; }

protected:
     void* _mapWritePbo();

    constDataHDDIOSPtr  _dataIOSPtr;
    PboSPtr             _pboSPtr;
    bool                _initialized;
    byte                _bytesNum; // bytes per voxel of original data
private:
    DecompressorBase(){}
};

} // namespace

#endif // MASS_VOL__COMPRESSION_H
