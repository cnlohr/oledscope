
const int adctarget = 400;

void ADC1_IRQHandler(void)
	__attribute__((interrupt));

void ADC1_IRQHandler(void)
{
	// Acknowledge pending interrupts.
	// This will always be ADC_JEOC, so we don't need to check.
	ADC1->STATR = 0;

	int adcraw = ADC1->RDATAR;

	// TODO: Use VDD.
	//int vdd = ADC1->IDATAR1;

	int err = (adctarget-adcraw);
	static int accum;

	if( err > 50 ) err = 50;
	if( err <-50 ) err =-50;

	accum += err>>2;
	if( accum > 8192 ) accum = 8192;
	if( accum < 0 ) accum = 0;
	int out = ((accum>>6) + err)>>5;
	if( out < 0 ) out = 0;
	const int limit = 6;
	// Limit

	int outlim = out;
	if( outlim < 0 ) outlim = 4;
	if( outlim > limit ) outlim = limit;

	TIM2->CH3CVR = outlim;

}

static void SetupADC()
{
	// Pin A2 is Analog we care about.
	funPinMode( PA2, GPIO_CFGLR_IN_ANALOG );

	// Reset the ADC to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

	// ADCCLK = 12 MHz => RCC_ADCPRE divide by 4
	RCC->CFGR0 &= ~RCC_ADCPRE;  // Clear out the bis in case they were set
	RCC->CFGR0 |= RCC_ADCPRE_DIV4;	// set it to 010xx for /4.

	// Set up single conversion on chl 0
	ADC1->RSQR1 = 0;
	ADC1->RSQR2 = 0;
	ADC1->RSQR3 = 0; // << This is what actually selects it.
	
	//Injection group is 8. NOTE: See note in 9.3.12 (ADC_ISQR) of TRM. The
	// group numbers is actually 4-group numbers.
	ADC1->ISQR = (8<<15) | (0<<20);

	// Sampling time for channels. Careful: This has PID tuning implications.
	// Note that with 3 and 3,the full loop (and injection) runs at 138kHz.
	ADC1->SAMPTR2 = (3<<(3*7)) | (3<<(3*8)); 
		// 0:7 => 3/9/15/30/43/57/73/241 cycles
		// (4 == 43 cycles), (6 = 73 cycles)  Note these are alrady /2, so 
		// setting this to 73 cycles actually makes it wait 256 total cycles
		// @ 48MHz.

	// Turn on ADC and set rule group to sw trig
	// * Enable Injection Trigger
	// * Trigger Injection off of ADC
	// * Enable ADC Trigger
	// * Trigger ADC off of Timer 2 TRGO
	ADC1->CTLR2 = ADC_ADON | ADC_JEXTTRIG | ADC_JEXTSEL | ADC_EXTTRIG | ADC_EXTSEL_1 | ADC_EXTSEL_0; 

	// Reset calibration
	ADC1->CTLR2 |= ADC_RSTCAL;
	while(ADC1->CTLR2 & ADC_RSTCAL);

	// Calibrate ADC
	ADC1->CTLR2 |= ADC_CAL;
	while(ADC1->CTLR2 & ADC_CAL);

	// enable the ADC Conversion Complete IRQ
	NVIC_EnableIRQ( ADC_IRQn );

	// ADC_JEOCIE: Enable the End-of-conversion interrupt.
	// ADC_JDISCEN | ADC_JAUTO: Force injection after rule conversion.
	// ADC_SCAN: Allow scanning.
	ADC1->CTLR1 = ADC_JEOCIE | ADC_JDISCEN | ADC_SCAN | ADC_JAUTO;

	// Same as above but no interrupts.
	//ADC1->CTLR1 = ADC_JDISCEN | ADC_SCAN | ADC_JAUTO;
}


void InitializePSU()
{
	RCC->APB2PCENR |= RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOA;
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;


	// Reset TIM2 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;
	
	// SMCFGR: default clk input is CK_INT
	// set TIM2 clock prescaler divider 
	TIM2->PSC = 0x0000;

	// set PWM total cycle width
	TIM2->ATRLR = 1023;

	TIM2->CHCTLR2 = TIM_OC3M_2 | TIM_OC3M_1 | TIM_OC3PE;
	TIM2->CCER = TIM_CC3E;// | TIM_CC3P;
	TIM2->CH3CVR = 0;  			// Actual duty cycle (Off to begin with)

	// Setup TRGO for ADC.  This makes is to the ADC will trigger on timer
	// reset, so we trigger at the same position every time relative to the
	// FET turning on.
	TIM2->CTLR2 = TIM_MMS_1;

	// Enable TIM1 outputs
	TIM2->BDTR = TIM_MOE;
	TIM2->CTLR1 = TIM_CEN | TIM_ARPE;

	// Setup remap.
	AFIO->PCFR1 |= AFIO_PCFR1_TIM2_REMAP_FULLREMAP;

	// Enable GPIOs appropriately.
	funPinMode( PD6, GPIO_CFGLR_OUT_50Mhz_AF_PP );


	// initialize counter
	TIM2->SWEVGR |= TIM_UG;

	SetupADC();

	Delay_Ms(1);
	while( ADC1->RDATAR < adctarget - 20 );

}

void PollPSU()
{
	printf( ".%d %d %d\n", ADC1->RDATAR, ADC1->IDATAR1, TIM2->CH3CVR );
}


