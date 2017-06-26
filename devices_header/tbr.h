/*
 * tbr.h
 *
 *  Created on: Mar 29, 2017
 *      Author: waseemh
 */

#ifndef SRC_TBR_H_
#define SRC_TBR_H_

#include "../drivers_header/rs485.h"
#include "../devices_header/rgb_led.h"

#define ARRAY_MESSAGE_SIZE		512
#define CMD_RX_TX_BUF_SIZE		256
#define	CMD_TX_BUF_SIZE			16


	/*
	 * private functions
	 */
void 		array_add(char data);
void		clear_buffer(char *buf, uint16_t size);
bool 		array_is_empty(void);
bool 		array_is_full(void);
bool 		check_other_messages(char * cmd_buf);
bool 		get_and_compare(char *compare_string);
uint8_t	  	CalculateLuhn(time_t * time);
char 		array_remove(void);
int			parse_message_tbr(char *buffer);
	/*
	 * public functions
	 */
void 		tbr_init(void);
void 		tbr_shutdown(void);
bool 		tbr_send_cmd(tbr_cmd_t tbr_cmd,time_t timestamp);
bool 		tbr_send_synch_msg(char *sync_msg);					//NOT used
uint8_t 	tbr_tag_count(void);
int 		tbr_recv_msg(char *msg_buf, int *msg_length);
uint8_t 	tbr_recv_msg_uint(uint8_t *lora_msg_buf, int *lora_length,
							char *msg_buf, int *msg_length);
#endif /* SRC_TBR_H_ */
