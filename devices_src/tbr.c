/*
 * tbr.c
 *
 *  Created on: Mar 30, 2017
 *      Author: waseemh
 */


#include "../devices_header/tbr.h"

	/*
	 * private variables
	 */
static	char		array_msg[ARRAY_MESSAGE_SIZE];
static 	int			array_size=0;
static 	int			array_front=0;
static 	int			array_rear=0;

	/*
	 * shared global varibales
	 */

	/*
	 * private functions
	 */
 bool array_is_full(void){
	 if(array_size==ARRAY_MESSAGE_SIZE){
		 return true;
	 }
	 else{
		 return false;
	 }
 }
 bool array_is_empty(void){
	 if(array_size==0){
		 return true;
	 }
	 else{
		 return false;
	 }
 }
 void array_add(char data){
	 if(array_size<ARRAY_MESSAGE_SIZE){
		 array_msg[array_rear]=data;
		 array_rear++;
		 array_size++;
		 if(array_rear==ARRAY_MESSAGE_SIZE){array_rear=0;}
	 }
 }
char array_remove(void){
	char	temp_char;
	 if(array_size>0){
		 temp_char=array_msg[array_front];
		 array_front++;
		 array_size--;
		 if(array_front==ARRAY_MESSAGE_SIZE){array_front=0;}
		 return temp_char;
	 }
	 else{
		 return '0';
	 }
}
uint8_t  CalculateLuhn(time_t * time){

	int digit, counter;
	uint8_t timeStampArray[9];
	uint32_t luhn_sum = 0;

	time_t timestamp = *time / 10; /* Cut last digit */

	/* Make an array of digits of the time stamp */
	for (counter = 9; counter >= 1; counter--){
		digit = timestamp % 10;
		timestamp = timestamp / 10;
		timeStampArray[counter - 1] = digit;
	}

	/* Calculate luhn sum */
	for (int i = 0; i < 9; i++){

		if (i % 2 == 0){
		timeStampArray[i] += timeStampArray[i];
		}

		if (timeStampArray[i] >= 10){
			luhn_sum += timeStampArray[i] % 10 + 1;
	}

		else luhn_sum += timeStampArray[i];
	}

	return (luhn_sum * 9) % 10 + '0';
}
int		parse_message_tbr(char *buffer){
	int			loop_var=0;
	char		*token_str;
	char		ref_token[2]="$";

	token_str=strtok(buffer,ref_token);
	if(token_str==NULL){
		return -1;
	}
	else{
		if(!array_is_full()){
			while(token_str!=NULL){
				array_add('$');
				for(loop_var=0;loop_var<strlen(token_str);loop_var++){
					 if(token_str[loop_var]=='\r'){
						 break;
					 }
					 array_add(token_str[loop_var]);
				}
				array_add('\n');
				token_str=strtok(NULL,ref_token);
			}
			return 1;
		}
		else{
			return -1;
		}
	}
}

bool check_other_messages(char * cmd_rx_tx_buf){
	int				temp_var=0;
	char	 		*cmd_compare_str;

	cmd_compare_str=strchr(cmd_rx_tx_buf,'$');
	 if(cmd_compare_str==NULL){
	 return false;
	 }
	 else{
		 temp_var=parse_message_tbr(cmd_rx_tx_buf);
		 if(temp_var>0){
			 return true;
		 }
		 else{
			 return false;
		 }
	 }
}
void clear_buffer(char *buf, uint16_t size){
	int			loop_var=0;
	for(loop_var=0;loop_var<size;loop_var++){
		buf[loop_var]=0;
	}
	return;
}
bool get_and_compare(char *compare_string){
	char 			*cmd_compare_str;
	char 			cmd_rx_tx_buf[CMD_RX_TX_BUF_SIZE];
	int				loop_var=0;
	char			temp_char='0';
	bool			ret_flag=false;
	bool			temp_flag=false;

	clear_buffer(cmd_rx_tx_buf,CMD_RX_TX_BUF_SIZE);
	delay_ms(7);												//response time from TBR
	for(loop_var=0;loop_var<FIFO_TBR_RX_DATA_SIZE;loop_var++){
		temp_char=rs485_recieve_char();
		if(temp_char=='@'){break;}
		cmd_rx_tx_buf[loop_var]=temp_char;
	}
	cmd_compare_str=strstr(cmd_rx_tx_buf,compare_string);
	if(cmd_compare_str!=NULL){
		ret_flag=true;
	}
	 else{
		 ret_flag=false;
	 }
	 temp_flag=check_other_messages(cmd_rx_tx_buf);
	 if(temp_flag==true){
		 rgb_on(false,false,true);
		 delay_ms(7);
		 rgb_shutdown();
	 }
	return ret_flag;
}
	////////////////////
char temp_buf[64];
///////////////////////
uint8_t convert_single_tbr_msg_into_uint(char *single_msg, uint8_t *dst_buf, uint8_t offset){
	char 			temp_buf[10];
	char			*temp_ptr;
	uint8_t			loop_var=0;
	uint32_t		timestamp=0;
	uint16_t		millisec=0;
	uint8_t			tagID=0;

	clear_buffer(temp_buf, 10);
		//extract timestamp
	for(loop_var=0;loop_var<10;loop_var++){
		temp_buf[loop_var]=single_msg[loop_var+8];
	}
	timestamp=(uint32_t)strtoul(temp_buf,&temp_ptr,16);
		//process and put into buffer
	dst_buf[offset+0]=(uint8_t)(timestamp>>3);
	dst_buf[offset+1]=(uint8_t)(timestamp>>2);
	dst_buf[offset+2]=(uint8_t)(timestamp>>1);
	dst_buf[offset+3]=(uint8_t)(timestamp>>0);

	sprintf(temp_buf,"\tConvert:TempStamp=%8x 0=%2x 1=%2x  3=%2x  4=%2x \n",timestamp,dst_buf[offset+0],dst_buf[offset+1],dst_buf[offset+2],dst_buf[offset+3]);
	debug_str(temp_buf);

	clear_buffer(temp_buf, 10);
		//extract millisec
	for(loop_var=0;loop_var<3;loop_var++){
		temp_buf[loop_var]=single_msg[loop_var+19];
	}
	millisec=(uint16_t)strtoul(temp_buf,&temp_ptr,16);
		//process and put into buffer
	dst_buf[offset+4]=(uint8_t)(millisec>>1);
	dst_buf[offset+5]=(uint8_t)(millisec>>0);
	clear_buffer(temp_buf, 10);
		//extract tagID
	for(loop_var=0;loop_var<2;loop_var++){
		temp_buf[loop_var]=single_msg[loop_var+28];
	}
	tagID=(uint8_t)strtoul(temp_buf,&temp_ptr,16);
		//process and put into buffer
	dst_buf[offset+6]=(uint8_t)(tagID>>0);
	return offset+7;		//fixed offset of 7bytes into last value.....
}
uint8_t convert_tbr_msgs_to_uint(char *src_buf, uint8_t *dst_buf, uint8_t msg_count){
	uint8_t 		inner_loop_var=0;
	uint8_t			outer_loop_var=0;
	uint8_t			offset_src_buf=0;
	uint8_t			offset_lora_buf=0;
	char 			single_msg[50];
	char			*temp_ptr;
	uint8_t			messages_converted=0;

	sprintf(temp_buf,"\tConvert:Msg Count=%d\n",msg_count);
	debug_str(temp_buf);

	clear_buffer(single_msg, 50);
		//extract and convert SN to uint8_t
	for(outer_loop_var=0;outer_loop_var<10;outer_loop_var++){
		if(src_buf[outer_loop_var+1]==','){break;}
		single_msg[outer_loop_var]=src_buf[outer_loop_var+1];	//+1 to ignore $
	}
	dst_buf[0]=(uint8_t)strtoul(single_msg,&temp_ptr,16);
	offset_lora_buf=1;
		//now convert rest of the messages into uint8_t (7 bytes per message => TimeStamp(4)+milli_sec(2)+tagID(1))
	offset_src_buf=0;
	for(outer_loop_var=0;outer_loop_var<msg_count;outer_loop_var++){
		clear_buffer(single_msg, 50);
		for(inner_loop_var=0;inner_loop_var<50;inner_loop_var++){
			single_msg[inner_loop_var]=src_buf[offset_src_buf+inner_loop_var];
			if(src_buf[offset_src_buf+inner_loop_var]=='\n'){break;}
		}
		offset_src_buf=inner_loop_var;
		if(strstr(single_msg,(const char*)"TBR Sensor")==NULL){		//only add detections NOT sensor values....
			offset_lora_buf=convert_single_tbr_msg_into_uint(single_msg,dst_buf,offset_lora_buf);
			messages_converted++;
		}
		debug_str("Message is:");
		debug_str(single_msg);
		debug_str("\n");
	}

	return messages_converted;
}
	/*
	 * public functions
	 */
void tbr_init(void){
	GPIO_PinModeSet(PWR_EN_PORT, RS485_12V_PWR_EN, gpioModePushPull, 0);
	rs485_init();
	rs485_enable();
	clear_buffer(array_msg,ARRAY_MESSAGE_SIZE);
	array_size=0;
	array_front=0;
	array_rear=0;
	return;
}

void tbr_shutdown(void){
	GPIO_PinOutClear(PWR_EN_PORT, RS485_12V_PWR_EN);
	return;
}

bool tbr_send_cmd(tbr_cmd_t tbr_cmd,time_t timestamp){
	char 			cmd_tx_buf[CMD_TX_BUF_SIZE];
	bool			ret_flag=false;
	time_t			my_timestamp;
	uint8_t			luhn;
	int				temp_var=0;

	if( tbr_cmd==cmd_sn_req){
		sprintf((char *)cmd_tx_buf,"?\n");
		rs485_transmit_string(cmd_tx_buf,1);
		ret_flag=get_and_compare((char *)"SN=");
	}
	else if(tbr_cmd==cmd_basic_sync){
		sprintf((char *)cmd_tx_buf,"(+)\n");
		rs485_transmit_string(cmd_tx_buf,3);
		ret_flag=get_and_compare((char *)"ack01");
	}
	else if(tbr_cmd==cmd_advance_sync){
		my_timestamp=timestamp;
		luhn=CalculateLuhn(&my_timestamp);
		sprintf((char *)cmd_tx_buf,"(+)%ld\n",my_timestamp);
		temp_var=strlen((const char *)cmd_tx_buf);
		cmd_tx_buf[temp_var-2]=luhn;						//change last digit of TimeStamp
		rs485_transmit_string(cmd_tx_buf,temp_var-1);
		ret_flag=get_and_compare((char *)"ack02");
	}
	else{
		ret_flag=false;
	}
	return ret_flag;
}
int tbr_recv_msg(char *msg_buf, int *msg_length){
	int 			msg_count=0;
	int 			loop_var=0;
	char			temp_char='0';

	clear_buffer(msg_buf, ARRAY_MESSAGE_SIZE);
	while(!array_is_empty()){
		temp_char=array_remove();
		if(temp_char=='$'){
			msg_count++;
		}
		msg_buf[loop_var]=temp_char;
		loop_var++;
	}
	*msg_length=loop_var;
	return msg_count;
}

int tbr_recv_msg_uint(uint8_t *lora_msg_buf, int *msg_length){
	int 			msg_count=0;
	int 			loop_var=0;
	char			temp_char='0';
	char 			msg_buf[ARRAY_MESSAGE_SIZE];

	clear_buffer(msg_buf, ARRAY_MESSAGE_SIZE);
	while(!array_is_empty()){
		temp_char=array_remove();
		if(temp_char=='$'){
			msg_count++;
		}
		msg_buf[loop_var]=temp_char;
		loop_var++;
	}
	temp_char=convert_tbr_msgs_to_uint(msg_buf,lora_msg_buf,(uint8_t)msg_count);
	*msg_length=loop_var;
	return msg_count;
}
