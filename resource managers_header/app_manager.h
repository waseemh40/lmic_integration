/*
 * app_manager.h
 *
 *  Created on: May 2, 2017
 *      Author: waseemh
 */

#ifndef SRC_APP_MANAGER_H_
#define SRC_APP_MANAGER_H_

#include "../drivers_header/delay.h"
#include "../devices_header/rgb_led.h"
#include "../devices_header/tbr.h"
#include "../drivers_header/rs232.h"
#include "../devices_header/ublox_gps.h"
#include "../fat_filesystem_header/ff.h"
#include "../fat_filesystem_header/diskio.h"
#include "../lmic/lmic.h"
#include "../hal/debug.h"
#include "../lmic/hal.h"

#define 	INIT_RETRIES	10

#define 	USE_TBR 			true
#define		USE_GPS				ture
#define		USE_SD_CARD			true
#define		USE_RADIO			true
#define 	BOTH_RADIO_SD_CARD	true		//app_manager.c => inside advance_synch_message function
//#define 	SD_CARD_ONLY		true		//app_manager.c => inside advance_synch_message function
//#define		RADIO_ONLY			true
//#define		USE_LORA_ACK		true
	/*
	 * shared variables
	 */

	/*
	 * private functions
	 */
bool 			file_sys_setup(uint16_t year,uint8_t date,uint8_t day,
											char buf[]);
bool 			tbr_cmd_update_rgb_led(tbr_cmd_t tbr_cmd, time_t timestamp);
	/*
	 * public functions
	 */
void 			app_manager_tbr_synch_msg(uint8_t time_manager_cmd, nav_data_t ref_timestamp,nav_data_t running_timestamp);
bool 			app_manager_init(void);
uint8_t			app_manager_get_lora_buffer(uint8_t	*lora_buffer);
nav_data_t 		app_manager_get_nav_data(void);

#endif /* SRC_APP_MANAGER_H_ */
