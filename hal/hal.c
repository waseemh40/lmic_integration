/*******************************************************************************
 * Copyright (c) 2014-2015 IBM Corporation.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    IBM Zurich Research Lab - initial API, implementation and documentation
 *******************************************************************************/
#include "../lmic/hal.h"
#include "../lmic/lmic.h"
#include "../drivers_header/pinmap.h"
#include "../devices_header/rgb_led.h"
#include "../drivers_header/spi.h"
#include "debug.h"
#include "em_emu.h"


// HAL state
static struct
{
    //int irqlevel;
    uint64_t ticks;
} HAL;
//////////////////////////////////////////////////////////////

static time_manager_cmd_t 		time_manager_cmd=basic_sync;
static int 						time_count=0;

void 		time_manager_init(void){
	//GPIO_PinModeSet(GPS_SIG_PORT, GPS_INT, gpioModeInput, 0);
	GPIO_PinModeSet(GPS_SIG_PORT, GPS_TIME_PULSE, gpioModeInput, 0);
	GPIO_IntConfig(GPS_SIG_PORT,GPS_TIME_PULSE,true,false,true);
	//GPIO_IntConfig(GPS_SIG_PORT,GPS_INT,true,false,false);
	GPIO_IntClear(_GPIO_IF_MASK);
	//GPIO_IntEnable();
    NVIC_EnableIRQ(GPIO_EVEN_IRQn);
    return;
}


unsigned long 		time_manager_unixTimestamp(int year, int month, int day,
              int hour, int min, int sec)
{
  const short days_since_beginning_of_year[12] = {0,31,59,90,120,151,181,212,243,273,304,334};

  int leap_years = ((year-1)-1968)/4
                  - ((year-1)-1900)/100
                  + ((year-1)-1600)/400;

  long days_since_1970 = (year-1970)*365 + leap_years
                      + days_since_beginning_of_year[month-1] + day-1;

  if ( (month>2) && (year%4==0 && (year%100!=0 || year%400==0)) )
    days_since_1970 += 1; /* +leap day, if year is a leap year */

  return sec + 60 * ( min + 60 * (hour + 24*days_since_1970) );
}

time_manager_cmd_t 		time_manager_get_cmd(void){
	return time_manager_cmd;
}
//////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// I/O
extern void radio_irq_handler(u1_t dio);
int gpio_int_flag=0;
void GPIO_EVEN_IRQHandler()	//impar
 {
	u4_t int_mask = GPIO_IntGetEnabled();
	GPIO_IntClear(int_mask);
	//debug_str("\tEven IRQ\n");
	if (int_mask & 1<<RADIO_IO_0){
		radio_irq_handler(0);
		//debug_str("\tEven IRQ 0\n");
		gpio_int_flag=2;
	}
	else if (int_mask & 1<<RADIO_IO_2){
		radio_irq_handler(2);
		//debug_str("\tEven IRQ 2\n");
		gpio_int_flag=2;
	}
	else if (int_mask & 1<<GPS_TIME_PULSE){
	     time_count++;
		 time_manager_cmd=3;
	     if(time_count%(BASIC_SYNCH_SECONDS)==0 && time_count!=ADVANCE_SYNCH_SECONDS){	//60
	         SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;
	    	 time_manager_cmd=basic_sync;
	     }
	     if(time_count==ADVANCE_SYNCH_SECONDS){
	    	 time_manager_cmd=advance_sync;
	    	 time_count=0;
	         SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;
	     }
	}
	//debug_str("\tEven IRQ END\n");
	return;
 }

void GPIO_ODD_IRQHandler()	//par
 {
	u4_t int_mask = GPIO_IntGetEnabled();
	GPIO_IntClear(int_mask);
	//debug_str("\tODD IRQ\n");
	if (int_mask & 1<<RADIO_IO_1){
		radio_irq_handler(1);
		//debug_str("\tODD IRQ 1");
		gpio_int_flag=1;
	}
	//debug_str("\tODD IRQ End\n");
	return;
 }

static void hal_io_init ()
{

	GPIO_PinModeSet(PWR_EN_PORT,RADIO_PWR_EN,gpioModePushPull,0);
	GPIO_PinOutSet(PWR_EN_PORT,RADIO_PWR_EN);
	spi_cs_set(radio);

	GPIO_PinModeSet(RADIO_IO_0345_PORT, RADIO_IO_0, gpioModeInput, 0);	//DIO0=PayLoadReady
	GPIO_PinModeSet(RADIO_IO_12_PORT, RADIO_IO_1, gpioModeInput, 0);	//DIO1=FifoLevel
	GPIO_PinModeSet(RADIO_IO_12_PORT, RADIO_IO_2, gpioModeInput, 0);	//DIO2=SyncAddr
	GPIO_PinModeSet(RADIO_IO_0345_PORT, RADIO_IO_3, gpioModeInput, 0);	//DIO3=FifoEmpty
	GPIO_PinModeSet(RADIO_IO_0345_PORT, RADIO_IO_4, gpioModeInput, 0);	//DIO4=PreambleDetect/RSSI
	GPIO_PinModeSet(RADIO_IO_0345_PORT, RADIO_IO_5, gpioModeInput, 0);	//DIO5=ModeReady

	GPIO_IntConfig(RADIO_IO_0345_PORT, RADIO_IO_0, true, false, true);
	GPIO_IntConfig(RADIO_IO_12_PORT, RADIO_IO_1, true, false, true);
	GPIO_IntConfig(RADIO_IO_12_PORT, RADIO_IO_2, true, false, true);

	GPIO_IntClear(_GPIO_IF_MASK);

	NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
	NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);

	NVIC_EnableIRQ(GPIO_ODD_IRQn);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}

void hal_pin_rxtx (u1_t val)
{
	//not used in PA52 nor PA53
}


// set radio NSS pin to given value
void hal_pin_nss (u1_t val)
{
	if (val)
		spi_cs_set(radio);
	else
		spi_cs_clear(radio);
}

// set radio RST pin to given value (or keep floating!)
void hal_pin_rst (u1_t val)
{
	return;
}

static void hal_spi_init ()
{

	spi_init();
	spi_enable();
}

// perform SPI transaction with radio
u1_t hal_spi (u1_t out)
{
	u1_t	ret_val=0;
	ret_val=spi_read_write_byte(out);
	return ret_val;
}


// -----------------------------------------------------------------------------
// TIME
static uint8_t       rtcInitialized = 0;    /**< 1 if rtc is initialized */
static uint32_t      rtcFreq;               /**< RTC Frequence. 32.768 kHz */

/***************************************************************************//**
 * @brief RTC Interrupt Handler, invoke callback function if defined.
 ******************************************************************************/
void RTC_IRQHandler(void)
{
	//debug_str("\tRTC IRQ");
	if (RTC_IntGet() & RTC_IF_OF)
	{
		HAL.ticks ++;
	}

    if(RTC_IntGet() & RTC_IF_COMP0) // expired
    {
        // do nothing, only wake up cpu
    }
	RTC_IntClear(_RTC_IF_MASK); // clear IRQ flags
}


static void hal_time_init ()
{
	RTC_Init_TypeDef init;

	rtcInitialized = 1;

	/* Ensure LE modules are accessible */
	CMU_ClockEnable(cmuClock_CORELE, true);

	/* Enable LFACLK in CMU (will also enable oscillator if not enabled) */
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

	/* Use the prescaler to reduce power consumption. */
	CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_1);

	rtcFreq = CMU_ClockFreqGet(cmuClock_RTC);

	/* Enable clock to RTC module */
	CMU_ClockEnable(cmuClock_RTC, true);

	init.enable   = false;
	init.debugRun = false;
	init.comp0Top = false;
	//init.comp0Top = true; /* Count to max before wrapping */
	RTC_Init(&init);

	/* Disable interrupt generation from RTC0 */
	RTC_IntDisable(_RTC_IF_MASK);
	RTC_IntEnable(RTC_IF_OF);	//Enable interrupt on overflow

	/* Enable interrupts */
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_EnableIRQ(RTC_IRQn);

	/* Enable RTC */
	RTC_Enable(true);
}

u4_t hal_ticks ()
{
    hal_disableIRQs();
    u4_t t = HAL.ticks;
    u4_t cnt = RTC_CounterGet();
	if (RTC_IntGet() & RTC_IF_OF)	// Overflow before we read CNT?
	{
        cnt = RTC_CounterGet();
        t ++;	// Include overflow in evaluation but leave update of state to ISR once interrupts enabled again
    }
    hal_enableIRQs();
	return (t<<24)|cnt;
}

// return modified delta ticks from now to specified ticktime (0 for past, FFFF for far future)
static u2_t deltaticks (u4_t time)
{
    u4_t t = hal_ticks();
    s4_t d = time - t;
    if( d<=0 ) return 0;    // in the past
    if( (d>>16)!=0 ) return 0xFFFF; // far ahead
    return (u2_t)d;
}

void hal_waitUntil (u4_t time)
{
	 while( deltaticks(time) != 0 ); // busy wait until timestamp is reached
}

// check and rewind for target time
u1_t hal_checkTimer (u4_t time)
{
    u2_t dt;
	RTC_IntClear(RTC_IF_COMP0);		//clear any pending interrupts
    if((dt = deltaticks(time)) < 5) // event is now (a few ticks ahead)
    {
    	RTC_IntDisable(RTC_IF_COMP0);	// disable IE
        return 1;
    }
    else // rewind timer (fully or to exact time))
    {
    	RTC_CompareSet(0, RTC_CounterGet() + dt);   // set comparator
    	RTC_IntEnable(RTC_IF_COMP0);  // enable IE
        return 0;
    }
}



// -----------------------------------------------------------------------------
// IRQ
static uint8_t irqlevel = 0;
static bool int_flag=false;

CORE_DECLARE_IRQ_STATE;
void hal_disableIRQs ()
{
	if(int_flag==false){
		int_flag=true;
		CORE_ENTER_ATOMIC();
	}
	 irqlevel++;

	return;

}

void hal_enableIRQs ()
{


	if(--irqlevel == 0) {
		CORE_EXIT_ATOMIC();
		int_flag=false;
	}
	return;
}

void hal_sleep ()
{
	//EMU_EnterEM2(false);
}

// -----------------------------------------------------------------------------

void hal_init ()
{

    memset(&HAL, 0x00, sizeof(HAL));
    hal_disableIRQs();

	hal_io_init();	// configure radio I/O and interrupt handler

    hal_spi_init();	// configure radio SPI

    hal_time_init();	// configure timer and interrupt handler

    hal_enableIRQs();
 }

void hal_failed ()
{
	rgb_on(true,false,false);
	CORE_EXIT_ATOMIC();
	debug_str("Failed");
	spi_cs_clear(radio);
	delay_ms(9);
	while(1){
	spi_write_byte(0x81);
	spi_write_byte(0x80);
	//delay_ms(1);
	spi_write_byte(0x01);
	if(spi_read_byte()==0x80){debug_str("OK");break;};
	delay_ms(9);
	}
	spi_cs_set(radio);
	NVIC_SystemReset();
	// HALT...
    hal_sleep();
    while(1);
}

