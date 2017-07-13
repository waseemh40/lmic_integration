/*
 * lpwan_manager.h
 *
 *  Created on: Jun 29, 2017
 *      Author: waseemh
 */

#ifndef RESOURCE_MANAGERS_HEADER_LPWAN_MANAGER_H_
#define RESOURCE_MANAGERS_HEADER_LPWAN_MANAGER_H_

#include "../lmic/hal.h"
#include "../resource managers_header/app_manager.h"
		/*
		 * Node parameters
		 */
	// LoRaWAN Application identifier (0xLSB, 0xxx, ......, 0xMSB)
static const u1_t APPEUI[8]  = {  0x88,	0x99,	0x11,	0x55,	0x44,	0x22,	0x11,	0x00};

	// LoRaWAN DevEUI, unique device ID (0xLSB, 0xxx, ......, 0xMSB)
static const u1_t DEVEUI[8]  = { 0x00,	0x81,	0x00,	0x00,	0x00,	0x00,	0x78,	0x86};

	//DEVKEY (0xMSB, 0xxx, ......, 0xLSB i.e. normal format)
static const u1_t DEVKEY[16] = {0x00,	0x11,	0x00,	0x22,	0x00,	0x33,	0x00,	0x44,	0x00,	0x55,	0x00,	0x66,	0x00,	0x77,	0x00,	0x88};

		/*
		 * public variables
		 */
extern	uint8_t 		lora_buffer[512];
extern	uint8_t			lora_msg_length;
extern	nav_data_t	 	ref_timestamp;


		/*
		 * public functions
		 */
void 	lpwan_init(void);
void 	onEvent (ev_t ev);

#endif /* RESOURCE_MANAGERS_HEADER_LPWAN_MANAGER_H_ */
