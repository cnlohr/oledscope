#include "ch32v003fun.h"
#include <stdio.h>
#include <string.h>
#include "sintable.h"

#define OLED_PORT C
#define OLED_DATA 6
#define OLED_CLOCK 5
#define OLED_DC 4
#define OLED_CS 2
#define OLED_RST 1


#define NOPBYTE 0xff // 0xbc is correct.  FF is for debugging.

// Reads in on rising clock, MSB first.

#define LOCAL_CONCAT( A, B) A##B
#define LOCAL_EXP_CONCATENATOR(A, B) LOCAL_CONCAT(A,B)
#define OLEDGPIO LOCAL_EXP_CONCATENATOR(GPIO, OLED_PORT)

#define OLED_PIN_TO( port, value ) { OLEDGPIO->BSHR = 1<<((!(value))*16 + (port)); }

// This determines the time between updates
#define SPI_DIVISOR 2 // Smaller = faster.
#define TIMER_DIVISOR 2 // Smaller = faster
#define DMA_BUFFER_LEN  32 // Write out this many command bytes per pixel location change.

// This is the buffer of points to go to.
#define NUM_CHAIN_ENTRIES 256

uint8_t spi_payload[DMA_BUFFER_LEN];
uint32_t chain_data[NUM_CHAIN_ENTRIES];

static void FillSPIPayload( uint8_t * buffer )
{
#if 1
	// Create grid to brr as fast as possible.
	static uint32_t ict = 7677;
		buffer[1] = 0xdc;
		buffer[0] = (ict >> 7) & 0x7f;
		buffer[3] = 0xd3;
		buffer[2] = ict & 0x7f;
	ict++;

#else
	static int x = 128*65536, y = 0;
	static int yspeed = 1;
	static int yspeed_direction = 0;
	static int ofs = 0;

	{
		//

		//ofs += 128;
		//SetPixelTo( sintable127[((x>>16)+ofs)&0x1ff], sintable127[((y>>16)+ofs)&0x1ff] );

		/*
			uint8_t ramgo[] = {
				0xdc, 0,
				0xd3, y,
				0xdc, x,
				0xbb, 0xbb,
			};
		*/
		
		buffer[1] = 0xdc;
		buffer[0] = sintable127[((x>>16)+ofs)&0x1ff];
		buffer[3] = 0xd3;
		buffer[2] = sintable127[((y>>16)+ofs)&0x1ff];
//		buffer[5] = 0xdc;
//		buffer[4] = sintable127[((x>>16)+ofs)&0x1ff];

		x+=65536; y += yspeed;

		#if 1
		if( yspeed_direction == 0 )
			yspeed++;
		else
			yspeed--;
		if( yspeed == 262144 ) yspeed_direction = 1;
		if( yspeed == 1 ) yspeed_direction = 0;

		#else
			yspeed = 65536;
		#endif
	}
#endif
}


static void WriteByte( uint32_t byte )
{
	int i;
	for( i = 0; i < 8; )
	{
		OLEDGPIO->BCR = 1<<OLED_CLOCK;
		OLED_PIN_TO( OLED_DATA, byte & 0x80 )
		byte <<= 1;
		i++;
		OLEDGPIO->BSHR = 1<<OLED_CLOCK;
	}
	OLEDGPIO->BCR = 1<<OLED_CLOCK;
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



int main()
{
	SystemInit();

	RCC->APB2PCENR |= LOCAL_EXP_CONCATENATOR( RCC_APB2Periph_GPIO, OLED_PORT );
	RCC->APB2PCENR |= RCC_APB2Periph_TIM1;


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
		0xa8, 0x7f, // Iterate over all 128 rows (Multiplex Ratio)
		0xd3, 0x00, // Set display offset // Where this appears on-screen  (Some displays will be different)
		0xd5, 0xf0, // Set precharge properties.  THIS IS A LIE  This has todo with timing.  <<< This makes it go brrrrrrrrr
		0xd9, 0x1d, // Set pre-charge period  (This controls brightness)
		0xdb, 0x35, // Set vcomh
		0xad, 0x80, // Set Charge pump
		0xa0, 0x00, // Default mapping

		0xd9, 0xff, // Override brightness.
		0x81, 0xff, // Set constrast

		0xaf, // Display on.
	};
	SendCommand( 0, commands, sizeof( commands ) );
	Delay_Ms( 2 );
	
	// NOTES ABOUT D5 / D9 -> D5 seems primary clock and divisor control.  D9-> Some other tuning.
	// DC -> ???? changing this seems to cause display to disable frequently.
	
	// Searching for some sort of "GO BRRR" flag.
	SendCommand( 0, (uint8_t[]){0xff, 0xff, 0xd5, 0xf0}, 4 );
	Delay_Ms( 2 );
	SendCommand( 0, (uint8_t[]){0xff, 0xff, 0xd9, 0xf0}, 4 );
	Delay_Ms( 2 );
	
	// With the above, it looks like there's about 16k points per second. With DMA SIZE of 64, and 6MHz bus.

	//SendCommand( 0, (uint8_t[]){0xb6, 0xf0}, 2 );
	//Delay_Ms( 2 );


	// XXX TODO: b0/dc does somettthinggg weeeeirdd

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
			
			// Make width double wide (to get some more brightness)
			data[pxloc>>3] = 1<<(pxloc&7);
			data[(pxloc+1)>>3] |= 1<<((pxloc+1)&7);  
		}
		SendCommand( 1, data, sizeof( data ) );
	}

	uint8_t force_two_row_mode[] = {
		0xa8, 0, // Set MUX ratio (Actually # of lines to scan) (But it's this + 1)
	};
	SendCommand( 0, force_two_row_mode, sizeof( force_two_row_mode ) );

	memset( spi_payload, NOPBYTE, sizeof( spi_payload ) );

	OLED_PIN_TO( OLED_CLOCK, 0 )
	OLED_PIN_TO( OLED_DC, 0 )
	OLED_PIN_TO( OLED_CS, 0 )


	// Enable DMA + Peripherals
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1;

	// MOSI, Configure GPIO Pin
	GPIOC->CFGLR &= ~(0xf<<(4*OLED_DATA));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*OLED_DATA);

	GPIOC->CFGLR &= ~(0xf<<(4*OLED_CLOCK));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*OLED_CLOCK);


	// Configure SPI 
	SPI1->CTLR1 = 
		SPI_NSS_Soft | SPI_CPHA_1Edge | SPI_CPOL_Low | SPI_DataSize_16b |
		SPI_Mode_Master | SPI_Direction_1Line_Tx |
		((SPI_DIVISOR)<<3); // Divisior = 16 (3<<3) (48/16 = 3MHz) 
			// Divisor  = 8 (2<<3) (48/8) = 6MHz

	SPI1->CTLR2 = SPI_CTLR2_TXDMAEN;
	SPI1->HSCR = 1;

	SPI1->CTLR1 |= CTLR1_SPE_Set;

	//SPI1->DATAR = (NOPBYTE) | ((NOPBYTE)<<8); // Start by sending nops..

	//DMA1_Channel3 is for SPI1TX
	DMA1_Channel3->PADDR = (uint32_t)&SPI1->DATAR;
	DMA1_Channel3->MADDR = (uint32_t)spi_payload;
	DMA1_Channel3->CNTR = DMA_BUFFER_LEN/2; // Number of unique uint16_t entries.
	DMA1_Channel3->CFGR  =
		DMA_M2M_Disable |		 
		//DMA_Priority_VeryHigh |
		DMA_Priority_Low |
		DMA_MemoryDataSize_HalfWord |
		DMA_PeripheralDataSize_HalfWord |
		DMA_MemoryInc_Enable |
		DMA_Mode_Circular | // OR DMA_Mode_Circular or DMA_Mode_Normal
		DMA_DIR_PeripheralDST |
		0;//DMA_IT_TC | DMA_IT_HT; // Transmission Complete + Half Empty Interrupts. 

	DMA1_Channel3->CFGR |= DMA_CFGR1_EN;
	
	

//uint8_t [DMA_BUFFER_LEN];
//uint32_t chain_data[32];

	//DMA1_Channel3 is for SPI1TX
	DMA1_Channel2->PADDR = (uint32_t)spi_payload;
	DMA1_Channel2->MADDR = (uint32_t)chain_data;
	DMA1_Channel2->CFGR  =
		DMA_M2M_Disable |		 
		DMA_Priority_Low |
		DMA_MemoryDataSize_Word |
		DMA_PeripheralDataSize_Word |
		DMA_MemoryInc_Enable |
		DMA_Mode_Circular | // OR DMA_Mode_Circular or DMA_Mode_Normal
		DMA_DIR_PeripheralDST |
		0;//DMA_IT_TC | DMA_IT_HT; // Transmission Complete + Half Empty Interrupts. 

	DMA1_Channel2->CFGR |= DMA_CFGR1_EN;
	DMA1_Channel2->CNTR = NUM_CHAIN_ENTRIES; // Number of unique uint32_t entries.


	// Setup DMA Channel 2 to refill buffer.
	// It's hooked to TIM1 CH1.

	// Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;
	TIM1->PSC = 16<<TIMER_DIVISOR;
	TIM1->ATRLR = DMA_BUFFER_LEN;
	TIM1->SWEVGR |= TIM_UG;
	TIM1->CCER |= TIM_CC1E;
	TIM1->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1;
	TIM1->CH1CVR = 128; // Trigger midway through.
	TIM1->BDTR |= TIM_MOE;
	TIM1->CTLR1 |= TIM_CEN;	
	TIM1->DMAINTENR = TIM_TDE | TIM_COMDE | TIM_UDE | TIM_CC1DE;

	for( i = 0; i < NUM_CHAIN_ENTRIES; i++ )
	{
		FillSPIPayload( &chain_data[i] );
	}
	
	int head = 0;
	int delta;
	while(1)
	{

		do
		{
			delta = ( NUM_CHAIN_ENTRIES * 2 - DMA1_Channel2->CNTR - head ) % NUM_CHAIN_ENTRIES;
			if( delta < 2 )
				break;

			FillSPIPayload( &chain_data[head] );
			head = (head+1)%NUM_CHAIN_ENTRIES;
		} while( 1 );
		Delay_Ms(4);
	}
}
