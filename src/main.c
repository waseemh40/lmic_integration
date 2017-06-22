

#include "../resource managers_header/app_manager.h"
#include "../lmic/hal.h"



////////////////////////////////////////////////////////////
	// CONFIGURATION (FOR APPLICATION CALLBACKS BELOW)
	//////////////////////////////////////////////////
	// LoRaWAN Application identifier (AppEUI)
	// Not used in this example
		//my gateway EUIs
	static const u1_t APPEUI[8]  = {  0x88,	0x99,	0x11,	0x55,	0x44,	0x22,	0x11,	0x00};

	// LoRaWAN DevEUI, unique device ID (LSBF)
	// Not used in this example
	static const u1_t DEVEUI[8]  = { 0x00,	0x80,	0x00,	0x00,	0x00,	0x00,	0x78,	0x86};

	// LoRaWAN NwkSKey, network session key
	// Use this key for The Things Network
	//static const u1_t DEVKEY[16] = {0x88,	0x00,	0x77,	0x00,	0x66,	0x00,	0x55,	0x00,	0x44,	0x00,	0x33,	0x00,	0x22,	0x00,	0x11,	0x00};
	static const u1_t DEVKEY[16] = {0x00,	0x11,	0x00,	0x22,	0x00,	0x33,	0x00,	0x44,	0x00,	0x55,	0x00,	0x66,	0x00,	0x77,	0x00,	0x88};
	//////////////////////////////////////////////////
	// APPLICATION CALLBACKS
	//////////////////////////////////////////////////

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
/////////////////////////////////////////////////////////////
	static int tx_function (void) {
		unsigned char buf[220];
		sprintf((char*)buf,"insh A ALLAH txt msg will be reveiced\n");// in good form and time will be much lesser I think so. Hello world this is Things Network for TTK8108 course...insh A ALLAH txt msg will be reveiced in good form\n");
		int channel=4;
		LMIC_setupBand(BAND_AUX,14,100);
		LMIC_setupChannel(4,868500000,DR_RANGE_MAP(DR_SF12,DR_SF7),BAND_AUX);
		for(int i=0; i<9; i++) { // For EU; for US use i<71
		  if(i != channel) {
		    LMIC_disableChannel(i);
		  }
		}
		LMIC_setDrTxpow(DR_SF7, 7);
		LMIC_setAdrMode(false);
		return (LMIC_setTxData2(2,buf,strlen((char*)buf),1));
	}

	static void initfunc (osjob_t* j) {
	    // reset MAC state
	    LMIC_reset();
	    // start joining
	    LMIC_startJoining();
	}
	static osjob_t blinkjob;
	static u1_t ledstate = 0;

	static void blinkfunc (osjob_t* j) {
		time_manager_cmd_t					time_manager_cmd=basic_sync;
		nav_data_t	 						nav_data;
	    // toggle LED
	    ledstate = !ledstate;
	    debug_led(ledstate);
	    // reschedule blink job
		time_manager_cmd=time_manager_get_cmd();
		if(time_manager_cmd==advance_sync) {
			  nav_data=app_manager_get_nav_data();
			  nav_data.gps_timestamp=time_manager_unixTimestamp(nav_data.year,nav_data.month,nav_data.day,
																nav_data.hour,nav_data.min,nav_data.sec);
			  app_manager_tbr_synch_msg(advance_sync,nav_data);
			  tx_function();
		}
		else if(time_manager_cmd==basic_sync){
			app_manager_tbr_synch_msg(basic_sync,nav_data);
		}
		else {
			;
		}
	   os_setCallback(j, blinkfunc);
	}

//////////////////////////////////////////////////////////////
	bool join_flag=false;
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
	bool								flag=false;
	const unsigned char  				rs232_tx_buf[64];


	flag=app_manager_init();
	if(flag){
		debug_str("\tApp Manager Init Successful\t\n");
	}
	else{
		debug_str("\tApp Manager Init Failed...\t\n");
		 rgb_on(true,false,false);
		 return 0;
	}
	time_manager_init();
	os_init();
	debug_str("OS initialized and join called. Waiting for join to finish...\n");

  while(1) {
		osjob_t initjob;
	    os_setCallback(&initjob, initfunc);
	    while(!join_flag);
	    // execute scheduled jobs and events
	    debug_str("\t\t\tJoined gateway. Starting system. Insh A ALLAH will be OK!\n");

	    os_setCallback(&initjob, blinkfunc);
	    os_runloop();
  }
/*	  SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
	  EMU_EnterEM1();
	  time_manager_cmd=time_manager_get_cmd();
	  if(time_manager_cmd==advance_sync) {
		  nav_data=app_manager_get_nav_data();
		  nav_data.gps_timestamp=time_manager_unixTimestamp(nav_data.year,nav_data.month,nav_data.day,
															nav_data.hour,nav_data.min,nav_data.sec);
	  }
	  app_manager_tbr_synch_msg(time_manager_cmd,nav_data);
   }
   */
}
//////////////////////////////////////////////////
// LMIC EVENT CALLBACK
//////////////////////////////////////////////////

void onEvent (ev_t ev) {
    //debug_event(ev);

    switch(ev) {

      // starting to join network
      case EV_JOINING:
          debug_str("EV_JOINING\n");
          break;

      // network joined, session established
      case EV_JOINED:
    	  debug_str("EV_JOINED\n");
    	  join_flag=true;
    	  //blinkfunc(&blinkjob);
          break;
      //transmission complete
      case EV_TXCOMPLETE:
    	  debug_str("EV_TXCOMPLETE. Going to sleep mode\n");
    	  //SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
    	  //EMU_EnterEM1();
    	  break;
      case EV_JOIN_FAILED:
    	  debug_str("EV_JOIN_FAILED\n");
    	  break;
      case EV_RXCOMPLETE:
    	  debug_str("EV_RXCOMPLETE\n");
    	  break;
      case EV_SCAN_TIMEOUT:
    	  debug_str("EV_SCAN_TIMEOUT\n");
    	  break;
      case EV_LINK_DEAD:
    	  debug_str("EV_LINK_DEAD\n");
	      break;
      case EV_LINK_ALIVE:
    	  debug_str("EV_LINK_ALIVE\n");
	      break;
      default:
    	  debug_str("Default Event..\n");
    	  break;
    }
}
