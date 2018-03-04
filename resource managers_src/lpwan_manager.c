/*
 * lpwan_manager.c
 *
 *  Created on: Jun 29, 2017
 *      Author: waseemh
 */

#include "../resource managers_header/lpwan_manager.h"

			/*
			 * Shared variables
			 */
	static osjob_t 			init_job;
	static osjob_t			app_job;
#ifdef USE_RADIO
	nav_data_t	 			running_tstamp;
	nav_data_t	 			ref_tstamp;
	int						diff_in_tstamp;
#endif
	char					temp_buf[128];
		////////////////////////////////////////
		uint32_t	one_sec_top_ref=32768;
	 	int			last_letimer_count=65535;
		uint16_t	average_n=0;
	 	uint32_t	avergae_sum=0;
	 	uint32_t	ref_count=0;
		//////////////////////////////////////////////
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

		 if(diff_in_tstamp!=0){
				sprintf(temp_buf,"\t\t\tTime Diff:Ref=%ld Cur=%ld diff=%d\t\n",(time_t)ref_tstamp.gps_timestamp,(time_t)running_tstamp.gps_timestamp,diff_in_tstamp);
				debug_str(temp_buf);
		 }
		sprintf(temp_buf,"\t\t\t\tSEC_TOP=%d,AVG=%d,AVG_N=%d,LAST=%d\n",one_sec_top_ref,avergae_sum,average_n,last_letimer_count);
		debug_str(temp_buf);
		debug_file_sys_setup((char *)"debug",temp_buf);

			//update Timestamps
		running_tstamp=gps_get_nav_data();
		running_tstamp.gps_timestamp=time_manager_unixTimestamp(running_tstamp.year,running_tstamp.month,running_tstamp.day,
																running_tstamp.hour,running_tstamp.min,running_tstamp.sec);
						//add 10secs
		ref_tstamp.gps_timestamp+=BASIC_SYNCH_SECONDS;
	    	//get synch command type from time manager
		time_manager_cmd=time_manager_get_cmd();
			//update application manager
		app_manager_tbr_synch_msg(time_manager_cmd,ref_tstamp,running_tstamp,diff_in_tstamp);
		if(time_manager_cmd==advance_sync){
			lora_msg_length=app_manager_get_lora_buffer(lora_buffer);
			if(lora_msg_length>0){
				sprintf(temp_buf,"LoRa message length=%d MSG=\n",lora_msg_length);
			 	debug_str((const u1_t*)temp_buf);
			 	for(int i=0;i<lora_msg_length;i++){
			 		sprintf(temp_buf,"%2x ",lora_buffer[i]);
			 	 	rs232_transmit_string(temp_buf,(uint8_t)strlen((char *)temp_buf));
					delay_ms(1);
			 	}
			 	debug_char('\n');
			 	lora_tx_function();
			}
			else{
			  sprintf(temp_buf,"No LoRa message\n");
			  debug_str((const u1_t*)"No LoRa message\n");
			}


		}
		os_clearCallback(&app_job);
	return;
	}
			/*
			 * public funtions
			 */

	void lpwan_init(void){
		os_init();
		debug_str((const u1_t*)"\t\tRadio Version. OS initialized and join called. Waiting for join to finish...\n");
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
			  while(1){
				  delay_ms(7);
				  ref_tstamp=gps_get_nav_data();
				  ref_tstamp.gps_timestamp=time_manager_unixTimestamp(ref_tstamp.year,ref_tstamp.month,ref_tstamp.day,
						  	  	  	  	  	  	  	  	  	  	  	  	 ref_tstamp.hour,ref_tstamp.min,ref_tstamp.sec);
				  if(ref_tstamp.valid==true && ref_tstamp.gps_timestamp%10==0){
					  break;
				  }
			  }
			  sprintf(temp_buf,"Ref Tstamp=%ld\n",ref_tstamp.gps_timestamp);
			  debug_str((const u1_t*)temp_buf);
			  ref_tstamp.gps_timestamp=ref_tstamp.gps_timestamp;

			  RMU_ResetControl(rmuResetBU, rmuResetModeClear);
			  time_manager_init();
			  sprintf(temp_buf,"Dstmp\tnano\tTstamp\tsec\tFlag\tTacc\tflags\n");
			  debug_str((const u1_t*)temp_buf);
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
