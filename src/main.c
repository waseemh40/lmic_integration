

#include "../resource managers_header/app_manager.h"
#include "../resource managers_header/lpwan_manager.h"

	/*
	 * Shared variables
	 */
uint8_t 		lora_buffer[512];
uint8_t			lora_msg_length=0;
nav_data_t	 	ref_timestamp;


int main() {
	 /*
	  ********************* Chip initialization*************
	  */
			CHIP_Init();
			CMU_HFRCOBandSet(cmuHFRCOBand_7MHz );
			CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
			CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
	 /*
	  *******************************************************
	  */

	if(app_manager_init()){
		debug_str((const u1_t*)"\tApp Manager Init Successful\t\n");
			//immediately update TimeStamp
		while(ref_timestamp.valid!=true){
			ref_timestamp=app_manager_get_nav_data();
			debug_str((const u1_t*)"\tWaiting for valid reference timestamp\t\n");
			delay_ms(7);
		}
		ref_timestamp.gps_timestamp=time_manager_unixTimestamp(ref_timestamp.year,ref_timestamp.month,ref_timestamp.day,
																ref_timestamp.hour,ref_timestamp.min,ref_timestamp.sec);

	}
	else{
		debug_str((const u1_t*)"\tApp Manager Init Failed...\t\n");
		 rgb_on(true,false,false);
		 return 0;
	}

	rgb_on(false,false,true);					//keep blue led on

  while(1) {
#ifdef USE_RADIO
	  lpwan_init();
#else
		time_manager_cmd_t		time_manager_cmd=basic_sync;
		nav_data_t	 			pos_nav_data;

		rgb_shutdown();
	    debug_str((const u1_t*)"\t\tNo radio version started\n");
		time_manager_init();
		while(1){
				//goto sleep
			SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
			EMU_EnterEM1();

				//update application manager
			time_manager_cmd=time_manager_get_cmd();
			pos_nav_data=app_manager_get_nav_data();
			ref_timestamp.gps_timestamp+=10;

			app_manager_tbr_synch_msg(time_manager_cmd,ref_timestamp);

		}
#endif
  }
}
