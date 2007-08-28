
#include "rawConverter.h"
#include "rawVolModel.h"
#include <iostream>
#include <fstream>
#include "hlp.h"

namespace eqVol
{

void lFailed( char* msg ) { EQERROR << msg << endl; }

void ConvertRawToRawPlusDerivatives( const string& src, const string& dst )
{
	uint32_t w, h, d;
//read header
	{
		string configFileName = src;
		hFile info( fopen( configFileName.append( ".vhf" ).c_str(), "rb" ) );
		FILE* file = info.f;

		if( file==NULL ) return lFailed( "Can't open header file" );

		fscanf( file, "w=%u\n", &w );
		fscanf( file, "h=%u\n", &h );
		fscanf( file, "d=%u\n", &d );
	}
	EQWARN << "Creating derivatives for raw model: " << src << " " << w << " x " << h << " x " << d << endl;

//read model	
	vector<uint8_t> volume( w*h*d, 0 );

	EQWARN << "Reading model" << endl;
	{
		ifstream file( src.c_str(), ifstream::in | ifstream::binary | ifstream::ate );
   
		if( !file.is_open() )
			return lFailed( "Can't open volume file" );

		ifstream::pos_type size;

		size = min( (int)file.tellg(), (int)volume.size() );

		file.seekg( 0, ios::beg );
		file.read( (char*)( &volume[0] ), size );	

		file.close();
	}
	
//calculate and save derivatives
	{
		EQWARN << "Calculating derivatives" << endl;
		ofstream file ( dst.c_str(), ifstream::out | ifstream::binary | ifstream::trunc );
   
		if( !file.is_open() )
			return lFailed( "Can't open destination volume file" );

	    int wh = w*h;

		vector<uint8_t> GxGyGzA( wh*d*4, 0 );
	
		for( uint32_t z=1; z<d-1; z++ )
	    {
	        int zwh = z*wh;

	        const uint8_t *curPz = &volume[0] + zwh;

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
		
		EQWARN << "Writing derivatives: " << dst.c_str() << " " << GxGyGzA.size() << " bytes" <<endl;
		file.write( (char*)( &GxGyGzA[0] ), GxGyGzA.size() );
		
		file.close();
	}
}

void SavToVhfConverter( const string& src, const string& dst )
{
	//read original header
	uint32_t w=1;
	uint32_t h=1;
	uint32_t d=1;
	float wScale=1.0;
	float hScale=1.0;
	float dScale=1.0;
	{
		hFile info( fopen( dst.c_str(), "rb" ) );
		FILE* file = info.f;
	
		if( file!=NULL )
		{
			fscanf( file, "w=%u\n", &w );
			fscanf( file, "h=%u\n", &h );
			fscanf( file, "d=%u\n", &d );

			fscanf( file, "wScale=%g\n", &wScale );
			fscanf( file, "hScale=%g\n", &hScale );
			fscanf( file, "dScale=%g\n", &dScale );
		}
	}
	
	//read sav
	int TFSize = 0;
	vector< uint8_t > TF( 256*4, 0 );
	{	
		hFile info( fopen( src.c_str(), "rb" ) );
		FILE* file = info.f;
	
		if( file==NULL ) return lFailed( "Can't open source sav file" );
	
		float t;
        int   ti;
		float tra;
		float tga;
		float tba;
		fscanf( file, "2DTF:\n"          );
		fscanf( file, "num=%d\n"    , &ti);
		fscanf( file, "mode=%d\n"   , &ti);
		fscanf( file, "rescale=%f\n", &t );
		fscanf( file, "gescale=%f\n", &t );
		fscanf( file, "bescale=%f\n", &t );
		fscanf( file, "rascale=%f\n", &t );
		fscanf( file, "gascale=%f\n", &t );
		fscanf( file, "bascale=%f\n", &t );
		fscanf( file, "TF:\n"            );
		fscanf( file, "res=%d\n",&TFSize );
		fscanf( file, "rescale=%f\n", &t );
		fscanf( file, "gescale=%f\n", &t );
		fscanf( file, "bescale=%f\n", &t );
		fscanf( file, "rascale=%f\n", &t );
		fscanf( file, "gascale=%f\n", &t );
		if( fscanf( file, "bascale=%f\n", &t ) != 1)
		    return lFailed( "failed to read header of sav file" );

		if( TFSize!=256  )  return lFailed( "Wrong size of transfer function, should be 256" );
		TF.resize( TFSize*4 );
		
		for( int i=0; i<TFSize; i++ )
		{
            fscanf( file, "re=%f\n", &t   ); TF[4*i  ] = hlpFuncs::clip<uint8_t>( t*255.0, 0, 255 );
			fscanf( file, "ge=%f\n", &t   ); TF[4*i+1] = hlpFuncs::clip<uint8_t>( t*255.0, 0, 255 );
			fscanf( file, "be=%f\n", &t   ); TF[4*i+2] = hlpFuncs::clip<uint8_t>( t*255.0, 0, 255 );
			fscanf( file, "ra=%f\n", &tra );    
			fscanf( file, "ga=%f\n", &tba );
            if( fscanf( file, "ba=%f\n", &tga ) !=1 )
            {
                EQERROR << "Failed to read entity #" << i << " of sav file" << endl;
                return;
            }
            TF[4*i+3] = hlpFuncs::clip<uint8_t>( (tra+tga+tba)*255.0/3.0, 0, 255 );
		}
		
	}
	
	//write vhf
	{
        {
            int i;
            memset( &TF[0], 0, 256*4 );
 			for (i=20; i<99; i++) {
				TF[(i*4)]  =  200;
				TF[(i*4)+1] = 200;
				TF[(i*4)+2] = 200;
			}
			for (i=20; i<49; i++) {
				TF[(i*4)+3] = 10;
			}
			for (i=50; i<99; i++) {
				TF[(i*4)+3] = 20;
			}
	
			for (i=100; i<255; i++) {
				TF[(i*4)]  =  93;
				TF[(i*4)+1] = 163;
				TF[(i*4)+2] = 80;
			}

			for (i=100; i<255; i++) {
				TF[(i*4)+3] = 255;
			}
      }
		hFile info( fopen( dst.c_str(), "wb" ) );
		FILE* file = info.f;
	
		if( file==NULL ) return lFailed( "Can't open destination header file" );
	
		fprintf( file, "w=%u\n", w );
		fprintf( file, "h=%u\n", h );
		fprintf( file, "d=%u\n", d );

		fprintf( file, "wScale=%g\n", wScale );
		fprintf( file, "hScale=%g\n", hScale );
		fprintf( file, "dScale=%g\n", dScale );
	
		fprintf( file,"TF:\n" );
	
		fprintf( file, "size=%d\n", TFSize );
	
		int tmp;
		for( int i=0; i<TFSize; i++ )
		{
			tmp = TF[4*i  ]; fprintf( file, "r=%d\n", tmp );
			tmp = TF[4*i+1]; fprintf( file, "g=%d\n", tmp );
			tmp = TF[4*i+2]; fprintf( file, "b=%d\n", tmp );
			tmp = TF[4*i+3]; fprintf( file, "a=%d\n", tmp );
		}
	}
    EQWARN << "file " << src.c_str() << " > " << dst.c_str() << " converted" << endl;
}

}


