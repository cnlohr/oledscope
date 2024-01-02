#include "ch32v003fun.h"
#include <stdio.h>
#include <string.h>

#define OLED_PORT C
#define OLED_DATA 6
#define OLED_CLOCK 5
#define OLED_DC 4
#define OLED_CS 2
#define OLED_RST 1

#include "rv003usb.h"

#define SCRATCH_SIZE 256 // In bytes

#define NOPBYTE 0xff
#define DO_PIXEL_BLANKING
#define NRPXW 1

// Reads in on rising clock, MSB first.

#define LOCAL_CONCAT( A, B) A##B
#define LOCAL_EXP_CONCATENATOR(A, B) LOCAL_CONCAT(A,B)
#define OLEDGPIO LOCAL_EXP_CONCATENATOR(GPIO, OLED_PORT)

#define OLED_PIN_TO( port, value ) { OLEDGPIO->BSHR = 1<<((!(value))*16 + (port)); }

// This determines the time between updates
#define SPI_DIVISOR 2 // Smaller = faster.
#define TIMER_DIVISOR 2 // Smaller = faster
#define DMA_BUFFER_LEN  46
	// Write out this many command bytes per pixel location change.  NOTE: This must be EVEN but, it should be based on reality doesn't have to be Pow2
	// 46 was experimentally found because beyond that the display start seriously dropping pixels.

// This is the buffer of points to go to.
#define NUM_CHAIN_ENTRIES 128 // In words

#define SCRATCH_MASK (SCRATCH_SIZE-1)
uint8_t scratch[SCRATCH_SIZE];
uint32_t scratch_head = 0;
uint32_t scratch_tail = 0;
uint8_t spi_payload[DMA_BUFFER_LEN];
uint32_t chain_data[NUM_CHAIN_ENTRIES];

static void FillSPIPayload( uint8_t * buffer )
{
	uint32_t local_tail = scratch_tail;
	uint32_t remain = (scratch_head - scratch_tail) & SCRATCH_MASK;
	if( remain > 2 ) // XXX WHYYYYYYYYYYYYYYYYYYY???????? Why not >1? ? I have noooo ideaaaaaa
	{
		// Create grid to brr as fast as possible.
		buffer[1] = 0xd3;
		buffer[0] = scratch[local_tail++] & 0x7f;
		buffer[3] = 0xdc;
		buffer[2] = scratch[local_tail++] & 0x7f;//(ict >> 7) & 0x7f;;
		
		scratch_tail = (local_tail) & SCRATCH_MASK;
	}
	else
	{
		static uint32_t ctr;
		ctr++;
		buffer[1] = 0xd3;
		buffer[0] = (ctr&1)?6:4; //ctr;
		buffer[3] = 0xdc;
		buffer[2] = (ctr&1)?6:4;
	}
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
	RCC->APB2PCENR |= RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOD; // PORT D for timer debugging

	usb_setup();

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
		0xdb, 0xff, // Set vcomh

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

		int pxloc = i-NRPXW;
		if( pxloc >= 0 )
		{
			
			// Make width double wide (to get some more brightness)
			int j;
			for( j = 0; j < NRPXW; j++ )
				data[(pxloc+j)>>3] |= 1<<((pxloc+j)&7);
		}
		SendCommand( 1, data, sizeof( data ) );
	}

	uint8_t force_two_row_mode[] = {
		0xa8, 0, // Set MUX ratio (Actually # of lines to scan) (But it's this + 1)  You can make this 1 for wider.
	};
	SendCommand( 0, force_two_row_mode, sizeof( force_two_row_mode ) );

	// Fill out the buffer with some data.
	memset( spi_payload, NOPBYTE, sizeof( spi_payload ) );
	#ifdef DO_PIXEL_BLANKING
		spi_payload[3] = 0xdc; // This enables blanking.
		spi_payload[2] = 0;
	#endif

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


	//DMA1_Channel3 is for SPI1TX
	DMA1_Channel3->PADDR = (uint32_t)&SPI1->DATAR;
	DMA1_Channel3->MADDR = (uint32_t)spi_payload;
	DMA1_Channel3->CNTR = DMA_BUFFER_LEN/2; // Number of unique uint16_t entries.
	DMA1_Channel3->CFGR  =
		DMA_M2M_Disable |		 
		DMA_Priority_Low |
		DMA_MemoryDataSize_HalfWord |
		DMA_PeripheralDataSize_HalfWord |
		DMA_MemoryInc_Enable |
		DMA_Mode_Circular | // OR DMA_Mode_Circular or DMA_Mode_Normal
		DMA_DIR_PeripheralDST |
		0;//DMA_IT_TC | DMA_IT_HT; // Transmission Complete + Half Empty Interrupts. 

//uint8_t [DMA_BUFFER_LEN];
//uint32_t chain_data[32];

	//DMA1_Channel3 is for SPI1TX
	DMA1_Channel2->PADDR = ((uint32_t)(spi_payload)) + 4;
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
	TIM1->PSC = 1;
	TIM1->ATRLR = (DMA_BUFFER_LEN*(8<<TIMER_DIVISOR))-1;
	TIM1->CCER = TIM_CC1E;
	TIM1->CH1CVR = TIM1->ATRLR/2; // Trigger midway through.
	TIM1->CTLR1 = TIM_CEN;
	TIM1->DMAINTENR = TIM_CC1DE;

	// PD2 is T1CH1, 10MHz Output alt func, push-pull  For debugging the timer.
#if 1
	TIM1->CCER |= TIM_CC1E | TIM_CC1P;
	TIM1->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1;	
	TIM1->BDTR |= TIM_MOE;
	GPIOD->CFGLR &= ~(0xf<<(4*2));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*2);
#endif

	for( i = 0; i < NUM_CHAIN_ENTRIES; i++ )
	{
		FillSPIPayload( (uint8_t*)&chain_data[i] );
	}

	// Starting shifting out frames.
	DMA1_Channel3->CFGR |= DMA_CFGR1_EN;

	
	int head = 0;
	int delta;
	while(1)
	{
		uint32_t * ue = GetUEvent();
		if( ue )
		{
			printf( "%lu %lx %lx %lx\n", ue[0], ue[1], ue[2], ue[3] );
		}
		
		do
		{
			delta = ( NUM_CHAIN_ENTRIES * 2 - DMA1_Channel2->CNTR - head ) % NUM_CHAIN_ENTRIES;
			if( delta < 2 )
				break;

			FillSPIPayload( (uint8_t*)&chain_data[head] );
			head = (head+1)%NUM_CHAIN_ENTRIES;
		} while( 1 );
		//printf( "SR: %d\n", start_write );
	}
}













/*
void usb_handle_user_in_request( struct usb_endpoint * e, uint8_t * scratchpad, int endp, uint32_t sendtok, struct rv003usb_internal * ist )
{
	// Make sure we only deal with control messages.  Like get/set feature reports.
	if( endp )
	{
		usb_send_empty( sendtok );
	}
}
*/

static uint32_t first_byte = 0;

void usb_handle_user_data( struct usb_endpoint * e, int current_endpoint, uint8_t * data, int len, struct rv003usb_internal * ist )
{
	int offset = e->count<<3;
	int torx = e->max_len - offset;
	//if( torx > len ) torx = len;
	//if( torx > 0 )
	{
		int available = (scratch_tail - scratch_head - 1) & SCRATCH_MASK;

		e->count++;
		uint8_t * dataend = data + len;

		// The first byte is the report ID.
		if( first_byte && len )
		{
			data++;
			first_byte = 0;
		}

		int local_scratch_head = scratch_head;
		while( data != dataend )
		{
			scratch[local_scratch_head] = *(data++);
			local_scratch_head = (local_scratch_head + 1) & SCRATCH_MASK;
		}
		scratch_head = local_scratch_head;

		if( available < 20 )
		{
			usb_send_data( 0, 0, 2, 0x5A ); // Send NACK (can't accept any more data right now)
			return;
		}
	}
	usb_send_data( 0, 0, 2, 0xD2 ); // Send ACK
	return;
}

void usb_handle_hid_get_report_start( struct usb_endpoint * e, int reqLen, uint32_t lValueLSBIndexMSB )
{
	e->opaque = 0;
	e->max_len = 0; // get report not supported yet.
}

void usb_handle_hid_set_report_start( struct usb_endpoint * e, int reqLen, uint32_t lValueLSBIndexMSB )
{
	scratch_head &= ~2; // Force each frame to be aligned to word-pair
	first_byte = 1;
	e->max_len = reqLen;
}


void usb_handle_other_control_message( struct usb_endpoint * e, struct usb_urb * s, struct rv003usb_internal * ist )
{
	LogUEvent( SysTick->CNT, s->wRequestTypeLSBRequestMSB, s->lValueLSBIndexMSB, s->wLength );
}


