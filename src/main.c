

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
	osjob_t 	initjob;;
	osjob_t		app_job;

	static u1_t ledstate = 0;

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
		debug_str("Tx Called\n");
		return (LMIC_setTxData2(2,buf,strlen((char*)buf),1));
	}

	static void initfunc (osjob_t* j) {
	    // reset MAC state
	    LMIC_reset();
	    // start joining
	    LMIC_startJoining();
	    //debug_str("joined called\n");
	}
		//jobs for os

	static void app_funct (osjob_t* j) {
		time_manager_cmd_t					time_manager_cmd=basic_sync;
		nav_data_t	 						nav_data;

			//goto sleep
		//debug_str("Going to sleep mode...\n");
		SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
		EMU_EnterEM1();
		debug_str("\tWoke up\n");
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
			onEvent(0);		//dummy event to sleep again....
		}
		else {
			;
		}
	}

//////////////////////////////////////////////////////////////
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

  while(1) {

		os_init();
		debug_str("OS initialized and join called. Waiting for join to finish...\n");
	    os_setCallback(&initjob, initfunc);
	    os_runloop();
  }
}
//////////////////////////////////////////////////
// LMIC EVENT CALLBACK
//////////////////////////////////////////////////

void onEvent (ev_t ev) {
    //debug_event(ev);

    switch(ev) {

      // starting to join network
      case EV_JOINING:
          debug_str("\tEV_JOINING\n");
          break;

      // network joined, session established
      case EV_JOINED:
    	  debug_str("\tEV_JOINED\n");
    	  app_funct(&app_job);	//first time call....
          break;
      //transmission complete
      case EV_TXCOMPLETE:
    	  debug_str("\tEV_TXCOMPLETE\n");
   	   os_setCallback(&app_job, app_funct);
    	  break;
      case EV_JOIN_FAILED:
    	  debug_str("\tEV_JOIN_FAILED\n");
    	  break;
      case EV_RXCOMPLETE:
    	  debug_str("\tEV_RXCOMPLETE\n");
    	  break;
      case EV_SCAN_TIMEOUT:
    	  debug_str("\tEV_SCAN_TIMEOUT\n");
    	  break;
      case EV_LINK_DEAD:
    	  debug_str("\tEV_LINK_DEAD\n");
	      break;
      case EV_LINK_ALIVE:
    	  debug_str("\tEV_LINK_ALIVE\n");
	      break;
      default:
    	  debug_str("\tDummy or default event..\n");
    	  os_setCallback(&app_job, app_funct);
    	  break;
    }
}
