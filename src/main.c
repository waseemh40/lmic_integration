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
int				diff_in_tstamp;
#endif

static 			unsigned char  		display_buffer[512];
static 			unsigned char  		rs232_tx_buf[512];


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
	init_led_switch();
	set_status_led(true,true);
	display_clear();
	sprintf(display_buffer,"Adddress=%2x\n",read_switch());
	display_put_string(6,3,display_buffer,font_medium);
	display_update();
	delay_ms(10);
	delay_ms(10);
	delay_ms(10);
	set_status_led(false,false);
	delay_ms(10);
	set_status_led(true,false);
	display_clear();
	display_put_string(3,3,"Init...\n\n",font_medium);
	display_update();

	if(app_manager_init()){
		debug_str((const u1_t*)"\tInit Successful\t\n");
		display_put_string(3,3,"\tSuccessful",font_medium);
		display_update();
	}
	else{
		debug_str((const u1_t*)"\tApp Manager Init Failed...\t\n");
		display_put_string(3,3,"\tFailed",font_medium);
		display_update();
		 return 0;
	}
	set_status_led(false,true);

#ifdef USE_RADIO
	delay_ms(10);
	delay_ms(10);
	display_clear();
	display_put_string(3,3,"\tJoininig LoRa\n",font_medium);
	display_update();
	lpwan_init();
#else
	delay_ms(10);
	delay_ms(10);
	display_clear();
	display_put_string(3,3,"Waiting GPS TS...",font_medium);
	display_update();
		time_manager_cmd_t		time_manager_cmd=basic_sync;
		uint8_t					gps_state=0;

		debug_str((const u1_t*)"\t\tNo radio version started\n");
		  while(1){
			  delay_ms(3);
			  ref_tstamp=gps_get_nav_data();
			  ref_tstamp.gps_timestamp=time_manager_unixTimestamp(ref_tstamp.year,ref_tstamp.month,ref_tstamp.day,
					  	  	  	  	  	  	  	  	  	  	  	  	 ref_tstamp.hour,ref_tstamp.min,ref_tstamp.sec);
			  if(ref_tstamp.fix==0x03 && ref_tstamp.gps_timestamp%10==0){
//			  if(ref_tstamp.fix==0x03){
				  break;
			  }
		  }
		display_clear();
		sprintf(display_buffer,"\tNo radio version\nExecuting loop...\nTS=%ld\n",ref_tstamp.gps_timestamp);
		display_put_string(3,3,display_buffer,font_medium);
		display_update();
		set_status_led(false,false);
		RMU_ResetControl(rmuResetBU, rmuResetModeClear);
		time_manager_init();
		while(1){
				//goto sleep
			SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
			EMU_EnterEM1();
					//add 10secs
			ref_tstamp.gps_timestamp+=BASIC_SYNCH_SECONDS;
					//get synch command type from time manager
			time_manager_cmd=time_manager_get_cmd();
					//update application manager
			app_manager_tbr_synch_msg(time_manager_cmd,ref_tstamp,running_tstamp,diff_in_tstamp);
			//update Timestamps
			gps_state=0;
			running_tstamp=gps_get_nav_data();
			if (running_tstamp.valid!=true){
				running_tstamp=gps_get_nav_data();
				gps_state=1;
			}
			if (running_tstamp.valid!=true){
				gps_state=2;
				running_tstamp=gps_get_nav_data();
			}
			running_tstamp.gps_timestamp=time_manager_unixTimestamp(running_tstamp.year,running_tstamp.month,running_tstamp.day,
																running_tstamp.hour,running_tstamp.min,running_tstamp.sec);

			/*if(time_manager_cmd==advance_sync){
				if(diff_in_tstamp!=0){
						sprintf(rs232_tx_buf,"\t\t\tTime Diff:Ref=%ld Cur=%ld diff=%d\t\n",(time_t)ref_tstamp.gps_timestamp,(time_t)running_tstamp.gps_timestamp,diff_in_tstamp);
						debug_str(rs232_tx_buf);
				}
			}*/
			sprintf(rs232_tx_buf,"\t\t\tTime Diff:Ref=%ld\tCur=%ld\tdiff=%d\tnano=%ld\tGPS_fix=%2x\tgps_state=%d\n",(time_t)ref_tstamp.gps_timestamp,(time_t)running_tstamp.gps_timestamp,diff_in_tstamp,running_tstamp.nano,running_tstamp.fix,gps_state);
			debug_str(rs232_tx_buf);
		}
#endif
  }
