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
	uint8_t buffer0[255] = { 0 }; // NOTE: This must be ONE MORE THAN what is in the hid descriptor.
	int r;
	int i;
	int j;
	int retries = 0;
	double dStart = OGGetAbsoluteTime();
	double dSecond = dStart;
	double dStartSend = 0.0;
	double dSendTotal = 0;


	uint32_t elementno;

//#define PAIRTEST
#define IIM_JULIA
//#define SIRPINSKI
#if defined( IIM_JULIA )
	double zreal = 0.1;
	double zimag = 0.1;
	double creal;
	double cimag;
#elif defined( SIRPINSKI )
	double sirpx = (rand()%1000) / 1000.0;
	double sirpy = (rand()%1000) / 1000.0;
#endif

	for( j = 0; ; j++ )
	{
		buffer0[0] = 0xaa; // First byte must match the ID.

#if defined( IIM_JULIA )
		creal = sin( j/100.0 )*1.0;
		cimag = cos( j/100.0 )*1.0;
#endif
		// But we can fill in random for the rest.
		for( i = 1; i < sizeof( buffer0 ); i+=2 )
		{
#if defined( PAIRTEST )
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
			buffer0[i+1] = (elementno>>7) & 0x7f;
			elementno++;
#endif
		}

		retrysend:
		
		dStartSend = OGGetAbsoluteTime();
		r = hid_send_feature_report( hd, buffer0, sizeof(buffer0) );
		dSendTotal += OGGetAbsoluteTime() - dStartSend;
		if( r != sizeof(buffer0) )
		{
			fprintf( stderr, "Warning: HID Send fault (%d) Retrying\n", r );
			retries++;
			if( retries > 10 ) break;
			goto retrysend;
		}

		retries = 0;
		printf( "<" ); // Print this out meaning we sent the data.

		if( dStartSend - dSecond > 1.0 )
		{
			printf( "\n%2.3f KB/s PC->003\n", j * .249 / dSendTotal );
			dSecond++;
		}

/*
		memset( buffer1, 0xff, sizeof( buffer1 ) );
		buffer1[0] = 0xaa; // First byte must be ID.

		double dStartRecv = OGGetAbsoluteTime();
		r = hid_get_feature_report( hd, buffer1, sizeof(buffer1) );
		dRecvTotal += OGGetAbsoluteTime() - dStartRecv;

		printf( ">" ); fflush( stdout);

		if( r != sizeof( buffer1 ) && r != sizeof( buffer1 ) + 1) { printf( "Got %d\n", r ); break; }

		// Validate the scratches matched.
		if( memcmp( buffer0, buffer1, sizeof( buffer0 ) ) != 0 ) 
		{
			printf( "%d: ", r );
			for( i = 0; i < r; i++ )
				printf( "[%d] %02x>%02x %s", i, buffer0[i], buffer1[i], (buffer1[i] != buffer0[i])?"MISMATCH ":""  );
			printf( "\n" );
			break;
		}
		
		*/
	}

	hid_close( hd );
}

