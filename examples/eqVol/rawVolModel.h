
#ifndef RAW_VOL_MODEL_H
#define RAW_VOL_MODEL_H

#include <eq/eq.h>

namespace eqVol
{

using namespace std;
using namespace eq;


#define COMPOSE_MODE_NEW


/** Just helping structure to automatically
    close files
*/
struct hFile
{
	hFile()             : f(NULL) {}
	hFile( FILE *file ) : f(file) {}
	~hFile() { if( f ) fclose( f ); }
	
	FILE *f;
};


/** Structure that contain actual dimensions of data that is 
    stored in volume texture.  

	It assumes that volume fills the cube [-1,-1,-1]..[1,1,1] and 
	the texture coordinates scaled to [0,0,0]..[1,1,1] but volume
	itself can be less then whole texture. Correct coordinates of
	volume stored in texture could be computed as:

	Xn = X * W
	Yn = Y * H
	Zn = Db + (Z - Do) * D
	
	so X and Y just scaled, but Z should be modifyed according to
	proper range.

*/
struct DataInTextureDimensions
{
	float W;    //!< Width  of data in texture (0..1]
	float H;    //!< Height of data in texture (0..1]
	float D;    //!< Depth  of data in texture (0..1]
	float Do;   //!< Depth offset (start of range)
	float Db;   //!< Depth border (necessary for preintegration)
};

/** Contain overal volume proportions relatively [-1,-1,-1]..[1,1,1] cube
*/
struct VolumeScales
{
	float W;
	float H;
	float D;
};

/** Load model to texture */
class RawVolumeModel
{
public:
	
    RawVolumeModel( const string& data );
	
	bool createTextures( GLuint &volume, GLuint &preint, Range range );

	      Range    getCurRange()   const { return _cRange;     };
	const string&  getFileName()   const { return _fileName;   };
	      uint32_t getResolution() const { return _resolution; };

//Data	
	DataInTextureDimensions TD;        //!< Data dimensions within volume texture 
	VolumeScales			volScales; //!< Proportions of volume
	
private:
	bool lSuccess() { return _lastSuccess=true;  }
	bool lFailed( char* msg )  { EQERROR << msg << endl; return _lastSuccess=false; }
	
	Range   _cRange;        //!< Current Range
	bool    _lastSuccess;   //!< Result of last loading

	uint32_t _resolution;   //!< max(w,h,d) of a model
	string  _fileName;
};

/*
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
*/

}

#endif // RAW_VOL_MODEL_H
