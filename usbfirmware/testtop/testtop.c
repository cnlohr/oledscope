#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// We borrow the combined hidapi.c from minichlink.
//
// This is for total perf testing.

#include "hidapi.c"

#include "os_generic.h"

int main()
{
	hid_device * hd = hid_open( 0x1209, 0xd003, L"CUSTOMDEVICE000"); // third parameter is "serial"
	if( !hd )
	{
		fprintf( stderr, "Error: Failed to open device.\n" );
		return -4;
	}

	// Size of buffers must match the report descriptor size in the special_hid_desc
	//  NOTE: You are permitted to have multiple entries.
	uint8_t buffer0[256] = { 0 }; // NOTE: This must be ONE MORE THAN what is in the hid descriptor.

// Override start status word.
#if 0
	buffer0[0] = 0xaa; // First byte must match the ID.
	buffer0[1] = 2; // Override start word.
	buffer0[2] = 0xff;
	buffer0[3] = 0xff;
	buffer0[4] = 0x00;
	buffer0[5] = 0xdc;
	hid_send_feature_report( hd, buffer0, sizeof(buffer0) );
#endif


	int r;
	int i;
	int j;
	int retries = 0;
	double dStart = OGGetAbsoluteTime();
	double dSecond = dStart;

	uint32_t elementno;

//#define PAIRTEST
//#define IIM_JULIA
//#define SIRPINSKI
//#define SWRITLE

#if defined( IIM_JULIA )
	double zreal = 0.1;
	double zimag = 0.1;
	double creal;
	double cimag;
#elif defined( SIRPINSKI )
	double sirpx = (rand()%1000) / 1000.0;
	double sirpy = (rand()%1000) / 1000.0;
#endif
	uint32_t pxno = 0;

	for( j = 0; ; j++ )
	{
		buffer0[0] = 0xaa; // First byte must match the ID.
		buffer0[1] = 0; // reserved for future use.

#if defined( IIM_JULIA )
		creal = sin( j/100.0 )*1.0;
		cimag = cos( j/100.0 )*1.0;
#endif

		//Sleep(100);

		// But we can fill in random for the rest.
		for( i = 2; i < sizeof( buffer0 )-2; i+=2 )
		{
			pxno++;
#if defined( SWRITLE )
			buffer0[i+0] = ( sin( pxno * 3.14159 * 0.00109  ) ) * 60 + 64;
			buffer0[i+1] = ( cos( pxno * 3.14159 * 0.00103 ) ) * 60 + 64;
#elif defined( PAIRTEST )

			// For seeing if there is xy leak.
			buffer0[i+0] = (i&2)?50 : 20;
			buffer0[i+1] = (i&2)?50 : 20;
#elif defined( IIM_JULIA )

			double real = zreal - creal;
			double imag = zimag - cimag;

			// Take square root of real & imag
			double mag = sqrt( real * real + imag * imag );
			zreal =                         sqrt( ( mag + real ) / 2.0 );
			zimag =  ((imag>=0)?1.0:-1.0) * sqrt( ( mag - real ) / 2.0 );
			if( rand() & 1 )
			{
				zreal = -zreal;
				zimag = -zimag;
			}
			//printf( "%f %f\n", zreal, zimag );

			buffer0[i+0] = (int)(zreal*32 + 64) & 0x7f;
			buffer0[i+1] = (int)(zimag*32 + 64) & 0x7f;


#elif defined( SIRPINSKI )
			// https://www.johndcook.com/blog/2017/07/08/the-chaos-game-and-the-sierpinski-triangle/
			// Sirpinski's gasket
			int k = rand()%3;
			const float corners[6] = { 0, 0, 0.5, 0.866, 1, 0 };
			sirpx = ( sirpx + corners[k*2+0] ) * 0.5;
			sirpy = ( sirpy + corners[k*2+1] ) * 0.5;

			buffer0[i+0] = (int)(sirpx*127) & 0x7f;
			buffer0[i+1] = (int)(sirpy*127 + 2) & 0x7f;
			//printf( "%d %f %f %f\n", k, sirpx, sirpy, corners[k*2+1]);
#else
			// Scanning line.
			buffer0[i+0] = elementno & 0x7f;
			buffer0[i+1] = ((elementno>>7)*10) & 0x7f;
			elementno++;
#endif
		}

		retrysend:
		
		r = hid_send_feature_report( hd, buffer0, sizeof(buffer0) );

		if( r != sizeof(buffer0) )
		{
			fprintf( stderr, "Warning: HID Send fault (%d) Retrying\n", r );
			retries++;
			if( retries > 10 ) break;
			goto retrysend;
		}

		retries = 0;
		printf( "<" ); // Print this out meaning we sent the data.

		double dNow = OGGetAbsoluteTime();
		if( dNow - dSecond > 1.0 )
		{
			printf( "\n%2.3f KB/s PC->003\n", j * .249 / (dNow-dStart) );
			dSecond++;
		}
	}

	hid_close( hd );
}

