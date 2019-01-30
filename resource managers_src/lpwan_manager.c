/*
 * lpwan_manager.c
 *
 *  Created on: Jun 29, 2017
 *      Author: waseemh
 */

#include "../resource managers_header/lpwan_manager.h"
#include "em_wdog.h"

			/*
			 * Shared variables
			 */
	static osjob_t 			init_job;
	static osjob_t			app_job;
	static bool				joined_lora=false;
	static bool				last_tx_complete=true;
	static uint8_t			gps_state=0;
	static uint8_t 			node_id=0;

#ifdef USE_RADIO
	nav_data_t	 			running_tstamp;
	nav_data_t	 			ref_tstamp;
	int						diff_in_tstamp;
#endif
	char					temp_buf[256];
			/*
			 * LMIC callbacks
			 */

	// provide application router ID (8 bytes, LSBF)
	void os_getArtEui (u1_t* buf) {
	    memcpy(buf, APPEUI, 8);
	}

	// provide device ID (8 bytes, LSBF)
	void os_getDevEui (u1_t* buf) {
	    memcpy(buf, DEVEUI, 8);
	    node_id=DEVEUI[1];		//which is MSByte -1!!!
	}

	// provide device key (16 bytes)
	void os_getDevKey (u1_t* buf) {
	    memcpy(buf, DEVKEY, 16);
	}

			/*
			 * private functions
			 */
	static	void lora_tx_function (void) {
#ifdef USE_LORA_ACK
		if(LMIC_setTxData2(2,lora_buffer,lora_msg_length,1)==0){ //Ack => blocking behavior....
			return;
		}
		else{
		 	debug_str((const u1_t*)"Tx function failed on length");
		 	//onEvent(0);
		}
#else
		if(!LMIC_setTxData2(2,lora_buffer,lora_msg_length,0)){
			return;
		}
		else{
		 	debug_str((const u1_t*)"Tx function failed on length");
		 	//onEvent(0);
		}
#endif
		return;
	}
	static void setup_channel (void){
		int 		channel=4;
			/*
			 * Channel settings:
			 * 868.5 MHz
			 * Spreading factor=7
			 * ADR=off
			 * 14dBm power @ 1% duty cycle
			 */
		LMIC_setupBand(BAND_AUX,14,100);
		LMIC_setupChannel(4,868500000,DR_RANGE_MAP(DR_SF12,DR_SF7),BAND_AUX);
		for(int i=0; i<9; i++) {
		  if(i != channel) {
			LMIC_disableChannel(i);
		  }
		}
		LMIC_setDrTxpow(DR_SF7, 14);
		LMIC_setAdrMode(false);
		return;
	}
	static void init_funct (osjob_t* j) {
	    // reset MAC state
	    LMIC_reset();
	    // start joining
	    LMIC_startJoining();
		return;
	}

	static void app_funct (osjob_t* j) {
		time_manager_cmd_t		time_manager_cmd=basic_sync;

				//add 10secs
		ref_tstamp.gps_timestamp+=BASIC_SYNCH_SECONDS;
				//get synch command type from time manager
		time_manager_cmd=time_manager_get_cmd();
				//update application manager
		app_manager_tbr_synch_msg(time_manager_cmd,ref_tstamp,running_tstamp,diff_in_tstamp);
					//update Timestamps
			gps_state=0;
			gps_poll_nav_status();
			while(gps_state<10){
				running_tstamp=gps_get_nav_data();
				if (running_tstamp.valid==true){
					break;
				}
					gps_state++;
			}
			running_tstamp.gps_timestamp=time_manager_unixTimestamp(running_tstamp.year,running_tstamp.month,running_tstamp.day,
																running_tstamp.hour,running_tstamp.min,running_tstamp.sec);

			diff_in_tstamp= (int)(ref_tstamp.gps_timestamp-running_tstamp.gps_timestamp);
			sprintf(temp_buf,"LoRA_join_flag=%d\tTime Diff:Ref=%ld\tCur=%ld\tdiff=%d\tmin=%d\tsec=%d\tnano=%ld\ttAcc=%ld\tGPS_fix=%2x\tgps_state=%d\n",joined_lora,(time_t)ref_tstamp.gps_timestamp,(time_t)running_tstamp.gps_timestamp,diff_in_tstamp,running_tstamp.min,running_tstamp.sec,running_tstamp.nano,running_tstamp.tAcc,running_tstamp.fix,gps_state);
			debug_str(temp_buf);
			if(time_manager_cmd==advance_sync && joined_lora==true && last_tx_complete==true){
				lora_msg_length=app_manager_get_lora_buffer(lora_buffer);
				lora_buffer[0]=node_id;
				if(lora_msg_length>0){
					lora_tx_function();
					last_tx_complete=false;
				}
			}
		WDOGn_Feed(WDOG);
		os_clearCallback(&app_job);
	return;
	}
			/*
			 * public funtions
			 */

	void lpwan_init(void){
		//Setup and initialize WDOG
	WDOG_Init_TypeDef	wdog_init=WDOG_INIT_DEFAULT;
	wdog_init.perSel=wdogPeriod_16k;
	wdog_init.em2Run=true;
	wdog_init.em3Run=true;
	CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);

  	  	  //Keep blue LED on
		rgb_on(false,false,true);
		RMU_ResetControl(rmuResetBU, rmuResetModeClear);
		gps_poll_nav_status();
		  while(1){
			  ref_tstamp=gps_get_nav_data();
			  ref_tstamp.gps_timestamp=time_manager_unixTimestamp(ref_tstamp.year,ref_tstamp.month,ref_tstamp.day,
					  	  	  	  	  	  	  	  	  	  	  	  	 ref_tstamp.hour,ref_tstamp.min,ref_tstamp.sec);
			  if(ref_tstamp.valid==false){
				  gps_state++;
				  if(gps_state>10){
					  debug_str("GPS double polling scenario\n");
					  delay_ms(7);
					  gps_poll_nav_status();
				  }
			  }
			  else {
				  if(ref_tstamp.fix==0x03 && ref_tstamp.gps_timestamp%10==0 && ref_tstamp.tAcc<=1000){
					  break;
				  }
				  else {
						gps_poll_nav_status();
				  }
			  }
		  }
		time_manager_init();
		os_init();
		WDOGn_Init(WDOG,&wdog_init);
		debug_str((const u1_t*)"\t\tRadio Version. OS and sync initialized. Waiting for join to finish...\n");
		os_setCallback(&init_job, init_funct);
		os_runloop();
	}

	void onEvent (ev_t ev) {

		switch(ev) {
		  case EV_JOINING:
			  debug_str((const u1_t*)"\tEV_JOINING\n");
			  break;
		  case EV_JOINED:
			  debug_str((const u1_t*)"\tEV_JOINED\n");
			  os_clearCallback(&init_job);
			  rgb_shutdown();
			  setup_channel();						//setup channel....
			  joined_lora=true;
			  break;
		  case EV_TXCOMPLETE:
#ifdef USE_LORA_ACK
		  if(LMIC.txrxFlags & TXRX_ACK){
			  debug_str((const u1_t*)"\tEV_TXCOMPLETE\n");
			  os_setCallback(&app_job, app_funct);
		  }
		  else{
			  debug_str((const u1_t*)"\nNo ACK RXCVD retrying...\n");
			  lora_tx_function();	//retry logic. NOT tested.
		  }
#else
		  debug_str((const u1_t*)"\tEV_TXCOMPLETE\n");
		  last_tx_complete=true;
#endif
			  break;
		  case EV_JOIN_FAILED:
			  debug_str((const u1_t*)"\tEV_JOIN_FAILED\n");
			  break;
		  case EV_RXCOMPLETE:
			  debug_str((const u1_t*)"\tEV_RXCOMPLETE\n");
			  break;
		  case EV_SCAN_TIMEOUT:
			  debug_str((const u1_t*)"\tEV_SCAN_TIMEOUT\n");
			  break;
		  case EV_LINK_DEAD:
			  debug_str((const u1_t*)"\tEV_LINK_DEAD\n");
			  break;
		  case EV_LINK_ALIVE:
			  debug_str((const u1_t*)"\tEV_LINK_ALIVE\n");
			  break;
		  default:
			  debug_str((const u1_t*)"\tEV_DEFAULT\n");
			  break;
		}
	}

	void debug_function(void){
		os_setCallback(&app_job, app_funct);
	}
