
#include "rawVolModel.h"
#include <iostream>
#include <fstream>
#include "hlp.h"

namespace eqVol
{
	
RawVolumeModel::RawVolumeModel()
:_width( 0 )
,_height(0 )
,_depth( 0 )
{
	_bbox.range[0] = 0;
	_bbox.range[1] = 0;
}

RawVolumeModel::RawVolumeModel( const string& filename, uint32_t w, uint32_t h, uint32_t d )
:_width( w )
,_height(h )
,_depth( d )
{
	_bbox.range[0] = 0;
	_bbox.range[1] = 0;
	
	_bbox.userData.resize( w*h*d );
	
	_bbox.transferFunc.resize( 4*256 );

	EQWARN << "Raw model: " << filename << " " << w << " x " << h << " x " << d << endl;
	
	ifstream file ( filename.c_str(), ifstream::in | ifstream::binary | ifstream::ate );
    
	if( !file.is_open() )
		return;

	ifstream::pos_type size;	
	
	size = min( (int)file.tellg(), (int)_bbox.userData.size() );
	
	file.seekg( 0, ios::beg );
	file.read( (char*)( &_bbox.userData[0] ), size );	
	
	file.close();		
}


RawVolumeModel::~RawVolumeModel()
{
}

void CreateTransferFunc( int t, uint8_t *transfer )
{
	memset( transfer, 0, 256*4 );

	int i;
	switch(t)
	{
		case 0: 
//spheres	
		EQWARN << "transfer: spheres" << endl;
			for (i=40; i<255; i++) {
				transfer[(i*4)]   = 115;
				transfer[(i*4)+1] = 186;
				transfer[(i*4)+2] = 108;
				transfer[(i*4)+3] = 255;
			}
			break;
	
		case 1:
// fuel	
			EQWARN << "transfer: fuel" << endl;
			for (i=0; i<65; i++) {
				transfer[(i*4)] = 255;
				transfer[(i*4)+1] = 255;
				transfer[(i*4)+2] = 255;
			}
			for (i=65; i<193; i++) {
				transfer[(i*4)]  =  0;
				transfer[(i*4)+1] = 160;
				transfer[(i*4)+2] = transfer[(192-i)*4];
			}
			for (i=193; i<255; i++) {
				transfer[(i*4)]  =  0;
				transfer[(i*4)+1] = 0;
				transfer[(i*4)+2] = 255;
			}
	
			for (i=2; i<80; i++) {
				transfer[(i*4)+3] = (unsigned char)((i-2)*56/(80-2));
			}	
			for (i=80; i<255; i++) {
				transfer[(i*4)+3] = 128;
			}
			for (i=200; i<255; i++) {
				transfer[(i*4)+3] = 255;
			}
			break;
			
		case 2: 
//neghip	
		EQWARN << "transfer: neghip" << endl;
			for (i=0; i<65; i++) {
				transfer[(i*4)] = 255;
				transfer[(i*4)+1] = 0;
				transfer[(i*4)+2] = 0;
			}
			for (i=65; i<193; i++) {
				transfer[(i*4)]  =  0;
				transfer[(i*4)+1] = 160;
				transfer[(i*4)+2] = transfer[(192-i)*4];
			}
			for (i=193; i<255; i++) {
				transfer[(i*4)]  =  0;
				transfer[(i*4)+1] = 0;
				transfer[(i*4)+2] = 255;
			}
	
			for (i=2; i<80; i++) {
				transfer[(i*4)+3] = (unsigned char)((i-2)*36/(80-2));
			}	
			for (i=80; i<255; i++) {
				transfer[(i*4)+3] = 128;
			}
			break;
	
		case 3: 
//bucky
		EQWARN << "transfer: bucky" << endl;
/*			for (i=20; i<255; i++) {
				transfer[(i*4)]  =  100;
				transfer[(i*4)+1] = 100;
				transfer[(i*4)+2] = 255;
				transfer[(i*4)+3] = 255;
			}
			break;
*/			
			for (i=20; i<99; i++) {
				transfer[(i*4)]  =  200;
				transfer[(i*4)+1] = 200;
				transfer[(i*4)+2] = 200;
			}
			for (i=20; i<49; i++) {
				transfer[(i*4)+3] = 10;
			}
			for (i=50; i<99; i++) {
				transfer[(i*4)+3] = 20;
			}
	
			for (i=100; i<255; i++) {
				transfer[(i*4)]  =  93;
				transfer[(i*4)+1] = 163;
				transfer[(i*4)+2] = 80;
			}

			for (i=100; i<255; i++) {
				transfer[(i*4)+3] = 255;
			}
			break;
	
		case 4: 
//hydrogen	
		EQWARN << "transfer: hydrogen" << endl;
			for (i=4; i<20; i++) {
				transfer[(i*4)] = 137;
				transfer[(i*4)+1] = 187;
				transfer[(i*4)+2] = 188;
				transfer[(i*4)+3] = 5;
			}

			for (i=20; i<255; i++) {
				transfer[(i*4)]  =  115;
				transfer[(i*4)+1] = 186;
				transfer[(i*4)+2] = 108;
				transfer[(i*4)+3] = 250;
			}
			break;
	
		case 5: 
//engine	
		EQWARN << "transfer: engine" << endl;
			for (i=100; i<200; i++) {
				transfer[(i*4)] = 	44;
				transfer[(i*4)+1] = 44;
				transfer[(i*4)+2] = 44;
			}
			for (i=200; i<255; i++) {
				transfer[(i*4)]  =  173;
				transfer[(i*4)+1] = 22;
				transfer[(i*4)+2] = 22;
			}
			// opacity

			for (i=100; i<200; i++) {
				transfer[(i*4)+3] = 8;
			}
			for (i=200; i<255; i++) {
				transfer[(i*4)+3] = 255;
			}
			break;
	
		case 6: 
//skull
		EQWARN << "transfer: skull" << endl;
			for (i=40; i<255; i++) {
				transfer[(i*4)]  =  128;
				transfer[(i*4)+1] = 128;
				transfer[(i*4)+2] = 128;
				transfer[(i*4)+3] = 128;
			}
			break;
	
		case 7: 
//vertebra
		EQWARN << "transfer: vertebra" << endl;
			for (i=40; i<255; i++) {
		        int k = i*2;
		        if (k > 255) k = 255;
				transfer[(i*4)]  =  k;
				transfer[(i*4)+1] = k;
				transfer[(i*4)+2] = k;
				transfer[(i*4)+3] = k;
			}	
			break;
	}
}

RawVolumeModel* RawVolumeModel::read( const string& data, const string& info )
{
	uint32_t w=0;
	uint32_t h=0;
	uint32_t d=0;
	uint32_t t=0;

	if( data.find( "spheres128x128x128"	, 0 ) != string::npos )
		t=0, w=h=d=128;
	
	if( data.find( "fuel"				, 0 ) != string::npos )
		t=1, w=h=d=64;
		
	if( data.find( "neghip"				, 0 ) != string::npos )
		t=2, w=h=d=64;
		
	if( data.find( "Bucky32x32x32"		, 0 ) != string::npos )
		t=3, w=h=d=32;
		
	if( data.find( "hydrogen"			, 0 ) != string::npos )
		t=4, w=h=d=128;
		
	if( data.find( "Engine256x256x256"	, 0 ) != string::npos )
		t=5, w=h=d=256;
		
	if( data.find( "skull"				, 0 ) != string::npos )
		t=6, w=h=d=256;
		
	if( data.find( "vertebra8"			, 0 ) != string::npos )
		t=7, w=h=512, d=256;
	
	if( w!=0 )
	{
		RawVolumeModel *model = new RawVolumeModel( data, w, h, d );
		CreateTransferFunc( t, &model->_bbox.transferFunc[0] );
		return model;
	}	

	return NULL;	
}

}
