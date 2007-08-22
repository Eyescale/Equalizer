
#ifndef RAW_VOL_MODEL_H
#define RAW_VOL_MODEL_H

#include <eq/eq.h>

using namespace std;

namespace eqVol
{
	
class RawVolumeModel
{
public:
	RawVolumeModel( const string& data, uint32_t w, uint32_t h, uint32_t d );
	RawVolumeModel();
	~RawVolumeModel();

	static RawVolumeModel* read( const string& data, const string& info );
	
	const uint8_t* GetData() 		 const { return &_bbox.userData[0]; }
	const uint8_t* GetTransferFunc() const { return &_bbox.transferFunc[0]; }
	
	uint32_t GetWidth()  const { return _width;  }
	uint32_t GetHeight() const { return _height; }
	uint32_t GetDepth()  const { return _depth;  }
	
private:
    struct BBox 
    {
        float range[2];

        vector<uint8_t> userData;
		vector<uint8_t> transferFunc;
    }_bbox;
	
	uint32_t _width; 
	uint32_t _height; 
	uint32_t _depth; 
};

}

#endif // RAW_VOL_MODEL_H
