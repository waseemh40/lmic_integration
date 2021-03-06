

#include "../resource managers_header/lpwan_manager.h"
#include "../resource managers_header/app_manager.h"

	/*
	 * Shared variables
	 */
#ifdef USE_RADIO
uint8_t 		lora_buffer[512];
uint8_t			lora_msg_length=0;
#else
nav_data_t	 	running_tstamp;
nav_data_t	 	ref_tstamp;
#endif

int main() {
	 /*
	  ********************* Chip initialization*************
	  */
			CHIP_Init();
			CMU_HFRCOBandSet(cmuHFRCOBand_7MHz );
			CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
			CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
			CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
	 /*
	  *******************************************************
	  */


	if(app_manager_init()){
		debug_str((const u1_t*)"\tApp Manager Init Successful\t\n");
	}
	else{
		debug_str((const u1_t*)"\tApp Manager Init Failed...\t\n");
		 rgb_on(true,false,false);
		 return 0;
	}
	/*
			  rgb_init();
			  rs232_init();
			  rs232_enable();
			  delay_init();
*/
	rgb_on(false,false,true);					//keep blue led on

  while(1) {
#ifdef USE_RADIO
	  lpwan_init();
#else
		time_manager_cmd_t		time_manager_cmd=basic_sync;

		rgb_shutdown();
		debug_str((const u1_t*)"\t\tNo radio version started\n");
		  while(ref_tstamp.valid!=true){		//wait for reference timestamp...
			  ref_tstamp=gps_get_nav_data();
			  delay_ms(5);
		  }
		ref_tstamp.gps_timestamp=time_manager_unixTimestamp(ref_tstamp.year,ref_tstamp.month,ref_tstamp.day,
				  	  	  	  	  	  	  	  	  	  	  	  ref_tstamp.hour,ref_tstamp.min,ref_tstamp.sec);
		RMU_ResetControl(rmuResetBU, rmuResetModeClear);
		time_manager_init();
		while(1){
				//goto sleep
			SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
			EMU_EnterEM1();
			//update Timestamps
			running_tstamp=gps_get_nav_data();
			running_tstamp.gps_timestamp=time_manager_unixTimestamp(running_tstamp.year,running_tstamp.month,running_tstamp.day,
																	running_tstamp.hour,running_tstamp.min,running_tstamp.sec);
			ref_tstamp.gps_timestamp+=BASIC_SYNCH_SECONDS;				//add 10secs
	    	//update application manager
			time_manager_cmd=time_manager_get_cmd();
			app_manager_tbr_synch_msg(time_manager_cmd,ref_tstamp,running_tstamp);

		}
#endif
  }
}
