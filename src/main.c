

#include "../resource managers_header/app_manager.h"
#include "../lmic/hal.h"

	/*
	 * Private shared variables
	 */
static		uint8_t 		lora_buffer[512];
static 		uint8_t			lora_msg_length=0;

	/*
	 * Private function declarations
	 */
static void 	setup_channel (void);
static void 	lora_tx_function (void);
static void 	app_funct (osjob_t* j);
static void 	init_func (osjob_t* j);

	///////////////////////////////////////////////////
	/////////////////LPWAN node parameters////////////
	//////////////////////////////////////////////////

	// LoRaWAN Application identifier (0xLSB, 0xxx, ......, 0xMSB)
	static const u1_t APPEUI[8]  = {  0x88,	0x99,	0x11,	0x55,	0x44,	0x22,	0x11,	0x00};

	// LoRaWAN DevEUI, unique device ID (0xLSB, 0xxx, ......, 0xMSB)
	static const u1_t DEVEUI[8]  = { 0x00,	0x80,	0x00,	0x00,	0x00,	0x00,	0x78,	0x86};

	//static const u1_t DEVKEY[16] (0xMSB, 0xxx, ......, 0xLSB i.e. normal format)
	static const u1_t DEVKEY[16] = {0x00,	0x11,	0x00,	0x22,	0x00,	0x33,	0x00,	0x44,	0x00,	0x55,	0x00,	0x66,	0x00,	0x77,	0x00,	0x88};

	//////////////////////////////////////////////////
	////////////////LMIC call backs///////////////////
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
	osjob_t 		init_job;;
	osjob_t			app_job;

	static void setup_channel (void){
		int 		channel=4;
			/*
			 * Channel settings:
			 * 868.5 MHz
			 * Spreading factor=7
			 * ADR=off
			 * 14dBm power @ 10% duty cycle
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

	static void lora_tx_function (void) {

		//sprintf((char*)lora_buffer,"insh A ALLAH txt msg will be reveiced\n");
		//debug_str("Tx Called\n");
		LMIC_setTxData2(2,lora_buffer,lora_msg_length,0); //Ack => blocking behavior....
		return;
	}

	static void init_func (osjob_t* j) {
	    // reset MAC state
	    LMIC_reset();
	    // start joining
	    LMIC_startJoining();
	    //debug_str("joined called\n");
		return;
	}
	static void app_funct (osjob_t* j) {
		time_manager_cmd_t		time_manager_cmd=basic_sync;
		nav_data_t	 			nav_data;

			//goto sleep
		SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
		EMU_EnterEM1();

	    	//update application manager
		time_manager_cmd=time_manager_get_cmd();
		if(time_manager_cmd==advance_sync) {
			  nav_data=app_manager_get_nav_data();
			  nav_data.gps_timestamp=time_manager_unixTimestamp(nav_data.year,nav_data.month,nav_data.day,
																nav_data.hour,nav_data.min,nav_data.sec);
			  app_manager_tbr_synch_msg(advance_sync,nav_data);
			  lora_msg_length=app_manager_get_lora_buffer(lora_buffer);
			  if(lora_msg_length>0){
				  debug_str((const u1_t*)"LoRa Tx Strtd\n");
				  lora_tx_function();
			  }
			  else{
				  debug_str((const u1_t*)"No LoRa Message\n");
				  onEvent(0);
				  //os_setCallback(&app_job, app_funct);
			  }
		}
		else if(time_manager_cmd==basic_sync){
			app_manager_tbr_synch_msg(basic_sync,nav_data);
			onEvent(0);
			//os_setCallback(&app_job, app_funct);
		}
		else {
			;
		}
		return;
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
	bool					flag=false;

	flag=app_manager_init();
	if(flag){
		debug_str((const u1_t*)"\tApp Manager Init Successful\t\n");
	}
	else{
		debug_str((const u1_t*)"\tApp Manager Init Failed...\t\n");
		 rgb_on(true,false,false);
		 return 0;
	}
	rgb_on(false,false,true);					//keep blue led on
  while(1) {

		os_init();
		debug_str((const u1_t*)"OS initialized and join called. Waiting for join to finish...\n");
	    os_setCallback(&init_job, init_func);
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
          debug_str((const u1_t*)"\tEV_JOINING\n");
          break;

      // network joined, session established
      case EV_JOINED:
    	  debug_str((const u1_t*)"\tEV_JOINED\n");
    	  rgb_shutdown();
    	  setup_channel();		//setup channel....
    	  time_manager_init();
    	  app_funct(&app_job);	//first time call....
          break;
      //transmission complete
      case EV_TXCOMPLETE:
    	  debug_str((const u1_t*)"\tEV_TXCOMPLETE\n");
    	  /*if(LMIC.txrxFlags & TXRX_ACK){
    		  //debug_str("\nAck Rxcvd\n");
        	  os_setCallback(&app_job, app_funct);
    	  }
    	  else{
    		  debug_str((const u1_t*)"\nNo ACK RXCVD retrying...\n");
    		  lora_tx_function();	//retry logic. NOT tested.
    	  }*/
    	  os_setCallback(&app_job, app_funct);
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
    	  //debug_str("\tDummy or default event..\n");
    	  os_setCallback(&app_job, app_funct);
    	  break;
    }
}
