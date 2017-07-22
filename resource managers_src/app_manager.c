/*
 * app_manager.c
 *
 *  Created on: May 2, 2017
 *      Author: waseemh
 */


#include "../resource managers_header/app_manager.h"

		/*
		 * private variables
		 */
static			FATFS 				FatFs;
static 			int					tbr_lora_length=0;
static 			uint8_t				tbr_lora_buf[ARRAY_MESSAGE_SIZE];

		/*
		 * public variables
		 */
const 			unsigned char  		rs232_tx_buf[128];

		/*
		 * private functions
		 */
bool file_sys_setup(uint16_t year,uint8_t month,uint8_t day, char buf[]){

	char 			filename[8]="00000000";
	bool			ret_flag=false;
	FIL  			f_pointer;
	FRESULT 		f_ret;

	/*
	 * FR_NOT_READY issue not solved with delay. Maybe send a command and wait...
	 * BUT only using f_open and not f_mount in this function works  :-)
	 * shutdown only once in init. NOT in this function
	 * Current Scenario: Current is 26mA on average but on write, goes to 46mA
	 * which is normal and as per logic. Function is almost done.
	 * Also works with remove and reinsert card
	 */
	sprintf(filename,"%4d%d%d.txt",year,month,day);
	f_ret = f_open(&f_pointer, filename, FA_WRITE | FA_OPEN_APPEND);
	if(f_ret==FR_OK){
    	f_ret = f_puts(buf,&f_pointer);
    	f_close(&f_pointer);
    	if(f_ret>0){
    		ret_flag=true;
    	}
    	else{
    		ret_flag=false;
    	}
    }
    else{
    	ret_flag=false;
		sprintf((char *)rs232_tx_buf,"f_open=%1d\n",f_ret);
		rs232_transmit_string((unsigned char *)rs232_tx_buf,strlen((const char *)rs232_tx_buf));
    	f_close(&f_pointer);
    }
    return ret_flag;
}
bool tbr_cmd_update_rgb_led(tbr_cmd_t tbr_cmd, time_t timestamp){
	bool 	ret_flag=false;

	ret_flag=tbr_send_cmd(tbr_cmd,timestamp);
	if(ret_flag){
		rgb_on(false,true,false);
		delay_ms(7);
	}
	else{
		rgb_on(true,false,false);
		delay_ms(7);
	}
	rgb_shutdown();
	return ret_flag;
}
		/*
		 * public functions
		 */
bool app_manager_init(void){
	  const unsigned char  		rs232_tx_buf[64];
	  uint8_t					init_retry=0;
	  bool						temp_init_flag=false;

	  	  	  	  //basic initializations
	  rgb_init();
	  rs232_init();
	  rs232_enable();
	  delay_init();
	  	  	  	  //Turn on init. LED (blue)
	  rgb_on(false,false,true);

#ifdef USE_GPS
	  init_retry=0;
	  do{
		  temp_init_flag= gps_init();
			 init_retry++;
			 if(init_retry>INIT_RETRIES){
					sprintf((char *)rs232_tx_buf,"GPS INIT FAILED\n");
					rs232_transmit_string(rs232_tx_buf,strlen((const char *)rs232_tx_buf));
					rgb_shutdown();
					rgb_on(true,false,false);
					return 0;
			 }
	  	  }while(!temp_init_flag);
	sprintf((char *)rs232_tx_buf,"GPS Init. DONE\n");
	rs232_transmit_string(rs232_tx_buf,strlen((const char *)rs232_tx_buf));
#endif

#ifdef USE_SD_CARD
	 do{
		 temp_init_flag=sd_card_init();
			 init_retry++;
			 if(init_retry>INIT_RETRIES){
					sprintf((char *)rs232_tx_buf,"Init. SD Card Failed\n");
					rs232_transmit_string((unsigned char *)rs232_tx_buf,21);
					rgb_shutdown();
					rgb_on(true,false,false);
					return 0;
			 }
		 }while(!temp_init_flag);

	init_retry=0;
	 do{
		 temp_init_flag=(bool)f_mount(&FatFs,"", 0);
			 init_retry++;
			 if(init_retry>INIT_RETRIES){
					sprintf((char *)rs232_tx_buf,"Mounting SD Card Failed\n");
					rs232_transmit_string((unsigned char *)rs232_tx_buf,24);
					sd_card_off();
					return false;
			 }
		 }while(temp_init_flag);
	//sd_card_off();
#else
	debug_str((const u1_t*)"\t\tNOT using SD Card\n");
#endif

#ifdef USE_TBR
	tbr_init();
	init_retry=0;
	 do{
		 temp_init_flag=	tbr_send_cmd(cmd_sn_req,(time_t) 0);
			 init_retry++;
			 if(init_retry>INIT_RETRIES){
					sprintf((char *)rs232_tx_buf,"TBR Missing....\n");
					rs232_transmit_string((unsigned char *)rs232_tx_buf,strlen((const char *)rs232_tx_buf));
					break;
			 }
		 }while(!temp_init_flag);
#endif
	 	 	 //Turn off init. LED
	 rgb_shutdown();
	 return true;
}

void app_manager_tbr_synch_msg(uint8_t  time_manager_cmd, nav_data_t ref_timestamp, nav_data_t running_tstamp){

	bool			temp_flag=false;
	int				tbr_msg_count=0;
	int				tbr_msg_length=0;
	char			tbr_msg_buf[ARRAY_MESSAGE_SIZE];

	if(time_manager_cmd==0){
		temp_flag=tbr_cmd_update_rgb_led(cmd_basic_sync,(time_t)ref_timestamp.gps_timestamp);
		sprintf((char *)rs232_tx_buf,"%ld\t%d\t%ld\t%d\t%d\t%ld\t%d\n",(time_t)running_tstamp.gps_timestamp,running_tstamp.nano,(time_t)ref_timestamp.gps_timestamp,running_tstamp.sec,temp_flag,running_tstamp.tAcc,running_tstamp.t_flags);
		rs232_transmit_string(rs232_tx_buf,strlen((const char *)rs232_tx_buf));
	}
	else if (time_manager_cmd==1){
		temp_flag=tbr_cmd_update_rgb_led(cmd_advance_sync,(time_t)ref_timestamp.gps_timestamp);
		sprintf((char *)rs232_tx_buf,"%ld\t%d\t%ld\t%d\t%d\t%ld\t%d\n",(time_t)running_tstamp.gps_timestamp,running_tstamp.nano,(time_t)ref_timestamp.gps_timestamp,running_tstamp.sec,temp_flag,running_tstamp.tAcc,running_tstamp.t_flags);
		rs232_transmit_string(rs232_tx_buf,strlen((const char *)rs232_tx_buf));
#ifdef SD_CARD_ONLY
	  tbr_msg_count=tbr_recv_msg((char *)tbr_msg_buf,&tbr_msg_length);
	  if(tbr_msg_count>0){
		temp_flag=file_sys_setup(ref_timestamp.year,ref_timestamp.month,ref_timestamp.day,tbr_msg_buf);
		sprintf((char *)rs232_tx_buf,"Wrt Flg=%1d Lngth=%3d\n",temp_flag,tbr_msg_length);
		//rs232_transmit_string(rs232_tx_buf,strlen((const char *)rs232_tx_buf));
	  }
#elif BOTH_RADIO_SD_CARD
	  tbr_msg_count=tbr_recv_msg_uint(tbr_lora_buf,&tbr_lora_length,tbr_msg_buf,&tbr_msg_length);
	  if(tbr_msg_count>0){
		  temp_flag=file_sys_setup(ref_timestamp.year,ref_timestamp.month,ref_timestamp.day,tbr_msg_buf);
		  sprintf((char *)rs232_tx_buf,"Wrt Flg=%1d Lngth=%3d Count=%d\n",temp_flag,tbr_msg_length,tbr_msg_count);
		//rs232_transmit_string(rs232_tx_buf,strlen((const char *)rs232_tx_buf));
	  }
#elif RADIO_ONLY
		  tbr_msg_count=tbr_recv_msg_uint(tbr_lora_buf,&tbr_lora_length,tbr_msg_buf,&tbr_msg_length);
#endif

	  }
	  else{
		  ;
	  }
	  /////////////////////////////////////////////////////////
	  sprintf((char *)rs232_tx_buf,"%ld\t%d\t%ld\t%d\t%d\t%ld\t%d\n",(time_t)running_tstamp.gps_timestamp,running_tstamp.nano,(time_t)ref_timestamp.gps_timestamp,running_tstamp.sec,temp_flag,running_tstamp.tAcc,running_tstamp.t_flags);
	  temp_flag=file_sys_setup(ref_timestamp.year,ref_timestamp.month,ref_timestamp.day,rs232_tx_buf);
	  //////////////////////////////////////////////////////////
	return;
}

nav_data_t app_manager_get_nav_data(void){
	return gps_get_nav_data();
}
uint8_t	app_manager_get_lora_buffer(uint8_t	*lora_buffer){
	int				loop_var=0;
	uint8_t			lora_buf_length=0;

	if(tbr_lora_length>0){
		for(loop_var=0;loop_var<tbr_lora_length;loop_var++){
			lora_buffer[loop_var]=tbr_lora_buf[loop_var];
		}
		lora_buf_length=(uint8_t)tbr_lora_length;
		tbr_lora_length=0;
		return lora_buf_length;
	}
	else{
		return 0;
	}
}
