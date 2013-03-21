//std12

#include "compression.h"

#include "../ramDataElement.h"
#include "../gpuCommands.h"
#include <eq/util/pixelBufferObject.h>
#include <msv/IO/dataHDDIO.h>

#include <msv/util/hlp.h> // cubed

//#undef EQ_USE_CUDA

namespace massVolVis
{


//----------------- DecompressorBase -----------------

DecompressorBase::DecompressorBase( constDataHDDIOSPtr dataIOSPtr, PboSPtr pboSPtr )
    : _dataIOSPtr( dataIOSPtr )
    , _pboSPtr(       pboSPtr )
    , _initialized( false     )
    , _bytesNum( 0 )
{
    if( _dataIOSPtr )
        _bytesNum = _dataIOSPtr->getBytesNum();
}

void* DecompressorBase::_mapWritePbo()
{
    return _pboSPtr ? _pboSPtr->mapWrite() : 0;
}


//----------------- DecompressorNone -----------------

class DecompressorNone : public DecompressorBase
{
public:
    DecompressorNone( constDataHDDIOSPtr dataIOSPtr, PboSPtr pboSPtr )
        : DecompressorBase( dataIOSPtr, pboSPtr ) { _initialized = true; }
    virtual ~DecompressorNone(){}

    virtual bool load( const RAMDataElement* dataEl, GPULoadRequest& request );
};

bool DecompressorNone::load( const RAMDataElement* dataEl, GPULoadRequest& request )
{
    if( !dataEl )
        return false;

    if( void* pboMem = _mapWritePbo( ))
    {
        uint32_t compressedSize = _dataIOSPtr->getBlockSize_( request.treePos );
        LBASSERT( compressedSize <= dataEl->size() );
        memcpy( pboMem, dataEl->data(), compressedSize );
        _pboSPtr->unmap();
        return true;
    }

    LBERROR << "PBO mapping failed: " << _pboSPtr->getError() << std::endl;
    return false;
}

//----------------- Decompressor selection -----------------

namespace
{
DecompressorSPtr _select( constDataHDDIOSPtr dataIOSPtr, constVolumeTreeBaseSPtr volumeTreeSPtr, PboSPtr pboSPtr, uint32_t capacity )
{
    if( !dataIOSPtr || !pboSPtr )
    {
        LBERROR << "Can't intialize decompressor" << std::endl;
        return DecompressorSPtr();
    }

    return DecompressorSPtr( new DecompressorNone( dataIOSPtr, pboSPtr ));
}
}

DecompressorSPtr Decompressors::select( constDataHDDIOSPtr dataIOSPtr, constVolumeTreeBaseSPtr volumeTreeSPtr, PboSPtr pboSPtr, uint32_t capacity )
{
    DecompressorSPtr decomp = _select( dataIOSPtr, volumeTreeSPtr, pboSPtr, capacity );
    if( decomp && !decomp->isIntialized() )
        return DecompressorSPtr();

    return decomp;
}

}
