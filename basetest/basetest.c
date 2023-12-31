#include "ch32v003fun.h"
#include <stdio.h>
#include "sintable.h"

#define OLED_PORT C
#define OLED_DATA 6
#define OLED_CLOCK 5
#define OLED_DC 4
#define OLED_CS 2
#define OLED_RST 1

// Reads in on rising clock, MSB first.

#define LOCAL_CONCAT( A, B) A##B
#define LOCAL_EXP_CONCATENATOR(A, B) LOCAL_CONCAT(A,B)
#define OLEDGPIO LOCAL_EXP_CONCATENATOR(GPIO, OLED_PORT)

#define OLED_PIN_TO( port, value ) { OLEDGPIO->BSHR = 1<<((!(value))*16 + (port)); }

static void WriteByte( uint32_t byte )
{
	int i;
	for( i = 0; i < 8; i++ )
	{
		OLEDGPIO->BCR = 1<<OLED_CLOCK;
		OLED_PIN_TO( OLED_DATA, byte & 0x80 )
		byte <<= 1;
		OLEDGPIO->BSHR = 1<<OLED_CLOCK;
	}
}

static void SendCommand( uint32_t is_data, const uint8_t * data, int len )
{
	int i;
	OLED_PIN_TO( OLED_DC, is_data )
	OLED_PIN_TO( OLED_CS, 0 )
	for( i = 0; i < len; i++)
	{
		WriteByte( data[i] );
	}
	OLED_PIN_TO( OLED_CS, 1 )
}

void SetPixelTo( int x, int y )
{
	uint8_t ramgo[] = {
		0xdc, 0,
		0xd3, y,
		0xdc, x,
	};
	SendCommand( 0, ramgo, sizeof( ramgo ) );
}

int main()
{
	SystemInit();

	// Enable GPIOs
	RCC->APB2PCENR |= LOCAL_EXP_CONCATENATOR( RCC_APB2Periph_GPIO, OLED_PORT );

	OLEDGPIO->CFGLR &= ~
		(
			(0xf<<(4*OLED_DATA)) |
			(0xf<<(4*OLED_CLOCK)) |
			(0xf<<(4*OLED_DC)) |
			(0xf<<(4*OLED_CS)) |
			(0xf<<(4*OLED_RST)) );
			
	OLEDGPIO->CFGLR |= 
		((GPIO_Speed_50MHz | GPIO_CNF_OUT_PP)<<(4*OLED_DATA)) |
		((GPIO_Speed_50MHz | GPIO_CNF_OUT_PP)<<(4*OLED_CLOCK)) |
		((GPIO_Speed_50MHz | GPIO_CNF_OUT_PP)<<(4*OLED_CS)) |
		((GPIO_Speed_50MHz | GPIO_CNF_OUT_PP)<<(4*OLED_DC)) |
		((GPIO_Speed_50MHz | GPIO_CNF_OUT_PP)<<(4*OLED_RST));

	OLEDGPIO->BCR = 1<<OLED_RST;
	OLEDGPIO->BSHR = 1<<OLED_CS;
	Delay_Ms( 1 );
	OLEDGPIO->BSHR = 1<<OLED_RST;
	Delay_Ms( 1 );


	// Initialize display.
	const uint8_t commands[] = {
		0xae, // Turn OLED off
		0x00, // Low column
		0x10, // High column
		0xb0, // Page address
		0xdc, 0x00, // Set Display Start Line  (Where in memory it reads from)
		0x81, 0x6f, // Set constrast
		0x21, // Set memory addressing mode
		0xa4, // normal (as opposed to invert colors, always on or off.)
		0xa8, 0x7f, // Multiplex Ratio, duty 1/64
		0xd3, 0x00, // Set display offset // Where this appears on-screen  (Some displays will be different)
		0xd5, 0x70, // Set precharge properties.  THIS IS A LIE  This has todo with timing.
		0xd9, 0x1d, // Set pre-charge period  (This controls brightness)
		0xdb, 0x35, // Set vcomh
		0xad, 0x80, // Set Charge pump
		0xa0, 0x00, // Default mapping

		0xd9, 0xff, // Override brightness.
		0x81, 0xff, // Set constrast

		0xaf, // Display on.
	};
	SendCommand( 0, commands, sizeof( commands ) );
	Delay_Ms( 250 );

	// XXX TODO: b0/dc does somettthinggg weeeeirddd


	// Force display on.
	if( 0 )
	{
		SendCommand( 0, (uint8_t[]){0xa5}, 1 );
		while(1);
	}

	// Write into RAM  (draw a diagonal line)
	int i = 0x00;
	for( i = 0; i < 128; i++ )
	{
		uint8_t ramwriteprepare[] = {
			0x75,
			0x00 | (i&0xf), // Row Low-address
			0x10 | (i>>4), // Row High-address
		};
		// Write out row of data
		SendCommand( 0, ramwriteprepare, sizeof( ramwriteprepare ) );
		uint8_t data[16] = { 0 };

		int pxloc = i-2;
		if( pxloc >= 0 )
		{
			data[pxloc>>3] = 1<<(pxloc&7);
			data[(pxloc+1)>>3] |= 1<<((pxloc+1)&7);
		}
		SendCommand( 1, data, sizeof( data ) );
	}

	uint8_t force_two_row_mode[] = {
		0xa8, 1, // Set MUX ratio (Actually # of lines to scan) (But it's this + 1)
	};
	SendCommand( 0, force_two_row_mode, sizeof( force_two_row_mode ) );

	int x = 128, y = 0;
	int yspeed = 1;
	int yspeed_direction = 0;
	int ofs = 0;
	while(1)
	{
		//ofs += 128;
		SetPixelTo( sintable127[((x>>16)+ofs)&0x1ff], sintable127[((y>>16)+ofs)&0x1ff] );
		Delay_Us( 200 );
		x+=65536; y += yspeed;

		if( yspeed_direction == 0 )
			yspeed++;
		else
			yspeed--;
		if( yspeed == 262144 ) yspeed_direction = 1;
		if( yspeed == 1 ) yspeed_direction = 0;
	}

/*
	while(1)
	{
		uint8_t update[] = {
			0xa8, 1, // Set MUX ratio (Actually # of lines to scan) (But it's this + 1)
			0xd3, 0,
			0xdc, 0,
		};
		update[3] = i&0x7f;
		update[5] = i&0x7f;
		SendCommand( 0, update, sizeof( update ) );
		Delay_Ms( 1 );
		i++;
	}
*/
}
