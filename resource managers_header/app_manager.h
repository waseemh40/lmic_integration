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
#include "../devices_header/radio.h"
#include "../fat_filesystem_header/ff.h"
#include "../fat_filesystem_header/diskio.h"
#include "../lmic/lmic.h"
#include "../hal/debug.h"
#include "../lmic/hal.h"

#define WRITE_SIZE		25
#define INIT_RETRIES	10

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
bool 			app_manager_init(void);
void 			app_manager_tbr_synch_msg(uint8_t time_manager_cmd, nav_data_t nav_data);
nav_data_t 		app_manager_get_nav_data(void);

#endif /* SRC_APP_MANAGER_H_ */
