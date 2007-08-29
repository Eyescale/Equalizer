
#include "rawVolModel.h"
#include <iostream>
#include <fstream>
#include "hlp.h"

namespace eqVol
{

#define COMPOSE_MODE_NEW


void createPreintegrationTable( const uint8_t *Table, GLuint &preintName )
{
	EQINFO << "Calculating preintegration table..." << endl;
			
	double rInt[256]; rInt[0] = 0.;
	double gInt[256]; gInt[0] = 0.;
	double bInt[256]; bInt[0] = 0.;
	double aInt[256]; aInt[0] = 0.;
	
	for( int i=1; i<256; i++ )
	{
		double tauc = ( Table[(i-1)*4+3] + Table[i*4+3] ) / 2.;
		
		rInt[i] = rInt[i-1] + ( Table[(i-1)*4+0] + Table[i*4+0] )/2.*tauc/255.;
		gInt[i] = gInt[i-1] + ( Table[(i-1)*4+1] + Table[i*4+1] )/2.*tauc/255.;
		bInt[i] = bInt[i-1] + ( Table[(i-1)*4+2] + Table[i*4+2] )/2.*tauc/255.;
		aInt[i] = aInt[i-1] + tauc;
	}

	vector<GLubyte> lookupI( 256*256*4, 255 );

	GLubyte* lookupImg = &lookupI[0];	// Preint Texture	
	int lookupindex=0;

	for( int sb=0; sb<256; sb++ )
	{
		for( int sf=0; sf<256; sf++ )
		{
			int smin, smax;
			if( sb<sf ) { smin=sb; smax=sf; }
			else        { smin=sf; smax=sb; }
			
			int rcol, gcol, bcol, acol;
			if( smax != smin )
			{
				double factor = 1. / (double)(smax-smin);
				rcol = (rInt[smax]-rInt[smin])*factor;
				gcol = (gInt[smax]-gInt[smin])*factor;
				bcol = (bInt[smax]-bInt[smin])*factor;
#ifdef COMPOSE_MODE_NEW
				acol = 256.*(exp(-(aInt[smax]-aInt[smin])*factor/255.));
#else
				acol = 256.*(1.0-exp(-(aInt[smax]-aInt[smin])*factor/255.));
#endif
			} else
			{
				double factor = 1./255.;
				rcol = Table[smin*4+0]*Table[smin*4+3]*factor;
				gcol = Table[smin*4+1]*Table[smin*4+3]*factor;
				bcol = Table[smin*4+2]*Table[smin*4+3]*factor;
#ifdef COMPOSE_MODE_NEW
				acol = 256.*(exp(-Table[smin*4+3]*1./255.));
#else
				acol = 256.*(1.0-exp(-Table[smin*4+3]*1./255.));
#endif
			}
			
            lookupImg[lookupindex++] = hlpFuncs::clip( rcol, 0, 255 );//min( rcol, 255 );	
			lookupImg[lookupindex++] = hlpFuncs::clip( gcol, 0, 255 );//min( gcol, 255 );
			lookupImg[lookupindex++] = hlpFuncs::clip( bcol, 0, 255 );//min( bcol, 255 );
			lookupImg[lookupindex++] = hlpFuncs::clip( acol, 0, 255 );//min( acol, 255 );
		}
	}
	
	
	glGenTextures( 1, &preintName );
	glBindTexture( GL_TEXTURE_2D, preintName );
	glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, lookupImg );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );
}

uint32_t getMinPow2( uint32_t size )
{
	uint32_t res=1;
    size--;
    
	while( size > 0 )
	{
		res  <<= 1;
		size >>= 1;
	}
	return res;
}

bool RawVolumeModel::createTextures( GLuint &volume, GLuint &preint, Range range )
{
	if( _cRange == range )
		return _lastSuccess;
		
	_cRange = range;
	
// reading header file & creating preintegration table
	uint32_t w, h, d;
	
    EQWARN << "--------------------------------------------" << endl;
    EQWARN << "loading model: " << _fileName;
	{
		string configFileName = _fileName;
		hFile info( fopen( configFileName.append( ".vhf" ).c_str(), "rb" ) );
		FILE* file = info.f;
	
		if( file==NULL ) return lFailed( "Can't open header file" );
	
		fscanf( file, "w=%u\n", &w );
		fscanf( file, "h=%u\n", &h );
		fscanf( file, "d=%u\n", &d );

		fscanf( file, "wScale=%g\n", &_VolScales.wScale );
		fscanf( file, "hScale=%g\n", &_VolScales.hScale );
		fscanf( file, "dScale=%g\n", &_VolScales.dScale );
	
        EQWARN << " " << w << "x" << h << "x" << d << endl;
        EQWARN << "width  scale = " << _VolScales.wScale << endl;
        EQWARN << "height scale = " << _VolScales.hScale << endl;
        EQWARN << "depth  scale = " << _VolScales.dScale << endl;

		if( fscanf(file,"TF:\n") !=0 ) return lFailed( "Error in header file" );
	
		uint32_t TFSize;
		fscanf( file, "size=%u\n", &TFSize );
	
		if( TFSize!=256  )  return lFailed( "Wrong size of transfer function, should be 256" );
	
		vector< uint8_t > TF( TFSize*4, 0 );
		int tmp;
		for( uint32_t i=0; i<TFSize; i++ )
		{
			fscanf( file, "r=%d\n", &tmp ); TF[4*i  ] = tmp;
			fscanf( file, "g=%d\n", &tmp ); TF[4*i+1] = tmp;
			fscanf( file, "b=%d\n", &tmp ); TF[4*i+2] = tmp;
			if( fscanf( file, "a=%d\n", &tmp ) != 1 )
			{
			    EQERROR << "Failed to read entity #" << i << " of TF from header file" << endl;
                break;
			}
			TF[4*i+3] = tmp;
		}

		createPreintegrationTable( &TF[0], preint );
	}

// reading volume and derivatives

    uint32_t s = static_cast<uint32_t>( hlpFuncs::clip<int32_t>( d*range.start, 0, d-1 ) );
    uint32_t e = static_cast<uint32_t>( hlpFuncs::clip<int32_t>( d*range.end-1, 0, d-1 ) );

	uint32_t start = static_cast<uint32_t>( hlpFuncs::clip<int32_t>( s-1, 0, d-1 ) );
	uint32_t end   = static_cast<uint32_t>( hlpFuncs::clip<int32_t>( e+2, 0, d-1 ) );
	uint32_t depth = end-start+1;

	uint32_t tW = getMinPow2( w ); 
	uint32_t tH = getMinPow2( h ); 
	uint32_t tD = getMinPow2( depth );
	
    EQWARN << "w: " << w << " " << tW << " h: " << h << " " << tH << " d: " << d << " " << depth << " " << tD << endl;

	vector<uint8_t> data( tW*tH*tD*4, 0 );

	_resolution = max( w, max( h, d ) );
    EQWARN << "r: " << _resolution << endl;
	
	_TD.W  =   (float)w / (float)tW;
	_TD.H  =   (float)h / (float)tH;
	_TD.D  = ( (float)(e-s+1) / (float)tD ) / ( range.end>range.start ? (range.end-range.start) : 1.0 );
	
	_TD.Do = range.start;
	_TD.Db = range.start > 0.0001 ? 1.0 / static_cast<float>(tD) : 0; // left  border in texture for depth

    EQWARN << "ws: " << _TD.W << " hs: " << _TD.H  << " wd: " << _TD.D << " Do: " << _TD.Do  << " Db: " << _TD.Db << endl << " s=" << start << " e=" << end << endl;

	{
		ifstream file ( _fileName.c_str(), ifstream::in | ifstream::binary | ifstream::ate );
    
		if( !file.is_open() ) return lFailed("Can't open model data file");

		file.seekg( w*h*start*4, ios::beg );
		
		if( w==tW && h==tH ) // width and height are power of 2
		{
			file.read( (char*)( &data[0] ), w*h*depth*4 );
		}else
			if( w==tW ) 	// only width is power of 2
			{
				for( uint32_t i=0; i<depth; i++ )
					file.read( (char*)( &data[i*tW*tH] ), w*h*4 );
			}else
			{				// nor width nor heigh is power of 2
				for( uint32_t i=0; i<depth; i++ )
					for( uint32_t j=0; j<h; j++ )
						file.read( (char*)( &data[i*tW*tH + j*tW] ), w*4 );
			}
	
		file.close();		
	}

// creating 3D texture

    glGenTextures( 1, &volume ); 	
	glBindTexture(GL_TEXTURE_3D, volume);
    
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );

    glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, tW, tH, tD, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)(&data[0]) );

	
	return lSuccess();
}


/*	

GLuint createGxGyGzATexture( const uint8_t *volume, uint32_t w, uint32_t h, uint32_t d, const eq::Range& range )
{
	EQINFO << "Precalculating gradients..." << endl;
	
    int wh = w*h;

	vector<uint8_t> GxGyGzA;
	GxGyGzA.resize( wh*d*4 );
	memset( &GxGyGzA[0], 0, wh*d*4 );
	
	uint32_t start = static_cast<uint32_t>( clip<int32_t>( d*range.start-1, 1, d-1 ) );
	uint32_t end   = static_cast<uint32_t>( clip<int32_t>( d*range.end  +2, 1, d-1 ) );
	
	for( uint32_t z=start; z<end; z++ )
    {
        int zwh = z*wh;

        const uint8_t *curPz = volume + zwh;

		for( uint32_t y=1; y<h-1; y++ )
        {
            int zwh_y = zwh + y*w;
            const uint8_t * curPy = curPz + y*w ;
			for( uint32_t x=1; x<w-1; x++ )
            {
                const uint8_t * curP = curPy +  x;
                const uint8_t * prvP = curP  - wh;
                const uint8_t * nxtP = curP  + wh;
				int32_t gx = 
                      nxtP[  1+w ]+ 3*curP[  1+w ]+   prvP[  1+w ]+
                    3*nxtP[  1   ]+ 6*curP[  1   ]+ 3*prvP[  1   ]+
                      nxtP[  1-w ]+ 3*curP[  1-w ]+   prvP[  1-w ]-
				
                      nxtP[ -1+w ]- 3*curP[ -1+w ]-   prvP[ -1+w ]-
                    3*nxtP[ -1   ]- 6*curP[ -1   ]- 3*prvP[ -1   ]-
                      nxtP[ -1-w ]- 3*curP[ -1-w ]-   prvP[ -1-w ];
				
				int32_t gy = 
                      nxtP[  1+w ]+ 3*curP[  1+w ]+   prvP[  1+w ]+
					3*nxtP[    w ]+ 6*curP[    w ]+ 3*prvP[    w ]+
					  nxtP[ -1+w ]+ 3*curP[ -1+w ]+   prvP[ -1+w ]-
					
				 	  nxtP[  1-w ]- 3*curP[  1-w ]-   prvP[  1-w ]-
					3*nxtP[   -w ]- 6*curP[   -w ]- 3*prvP[   -w ]-
                      nxtP[ -1-w ]- 3*curP[ -1-w ]-   prvP[ -1-w ];
				
				int32_t gz = 
                      prvP[  1+w ]+ 3*prvP[  1   ]+   prvP[  1-w ]+
                    3*prvP[    w ]+ 6*prvP[  0   ]+ 3*prvP[   -w ]+
					  prvP[ -1+w ]+ 3*prvP[ -1   ]+   prvP[ -1-w ]-
					
                      nxtP[  1+w ]- 3*nxtP[  1   ]-   nxtP[  1-w ]-
					3*nxtP[   +w ]- 6*nxtP[  0   ]- 3*nxtP[   -w ]-
					  nxtP[ -1+w ]- 3*nxtP[ -1   ]-   nxtP[ -1-w ];
				
				
				int32_t length = sqrt( (gx*gx+gy*gy+gz*gz) )+1;
				
				gx = ( gx*255/length + 255 )/2; 
				gy = ( gy*255/length + 255 )/2;
				gz = ( gz*255/length + 255 )/2;
				
				GxGyGzA[(zwh_y + x)*4   ] = (uint8_t)(gx);
				GxGyGzA[(zwh_y + x)*4 +1] = (uint8_t)(gy);
				GxGyGzA[(zwh_y + x)*4 +2] = (uint8_t)(gz);	
				GxGyGzA[(zwh_y + x)*4 +3] = curP[0];
			}
		}
	}
	
	EQINFO << "Done precalculating gradients." << endl;
	
	GLuint tex3D;
    glGenTextures( 1, &tex3D ); 	
	glBindTexture(GL_TEXTURE_3D, tex3D);
    
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );

    glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, w, h, d, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)(&GxGyGzA[0]) );

	return tex3D;
}
	
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
*/

}
