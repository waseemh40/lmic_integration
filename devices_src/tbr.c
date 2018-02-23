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
	char		temp_buffer[50];
	int			temp_buf_index=1;

	delay_ms(5);
	debug_str("\t\t\tTBR Parse Called\n");

	token_str=strtok(buffer,ref_token);
	if(token_str==NULL){
		debug_str("\t\t\tTBR Parse Returned\n");
		return -1;
	}
	else{
		if(!array_is_full()){
			while(token_str!=NULL){
				//array_add('$');
				temp_buffer[0]='$';
				for(loop_var=0;loop_var<strlen(token_str);loop_var++){
					 /*if(token_str[loop_var]=='\r'){
						 debug_str("\t\t\tParse Found One Message\n");
						 temp_buf_index=1;
						 break;
					 }*/
					 //array_add(token_str[loop_var]);
					 if(token_str[loop_var]=='\r'){
						 if(strlen(temp_buffer)>30){
							 for(loop_var=0;loop_var<temp_buf_index;loop_var++){
								 array_add(temp_buffer[loop_var]);
							 }
							 array_add('\n');
							 //debug_str("\t\t\tParse Found One Message\n");
							 //debug_str(temp_buffer);
							 //debug_str("\n");
							 //delay_ms(4);
						 }
						 else{
							 debug_str("\t\t\tParse Broken Message Found\n");
						 }
						 temp_buf_index=1;
						 break;
					 }
					 temp_buffer[temp_buf_index]=token_str[loop_var];
					 temp_buf_index++;
					 //////////////////////////
					 if(temp_buf_index>50){
						 temp_buf_index=50;
						 debug_str("\t\t\tParse temp_buf_index greater than 50, setting it to 50!!!\n");
						 delay_ms(3);
					 }
					 if(strlen(token_str)>512){
						 debug_str("\t\t\tParse strlen(token_strn) is greater than 512!!!\n");
						 delay_ms(3);
					 }
					 /////////////////////////
				}
				//array_add('\n');
				token_str=strtok(NULL,ref_token);
			}
			debug_str("\t\t\tTBR Parse Returned\n");
			return 1;
		}
		else{
			debug_str("\t\t\tTBR Parse Returned\n");
			return -1;
		}
	}
	return 0;
}

bool check_other_messages(char * cmd_rx_tx_buf){
	int				temp_var=0;
	char	 		*cmd_compare_str;

	delay_ms(5);
	debug_str("\t\tTBR CoM Called\n");

	cmd_compare_str=strchr(cmd_rx_tx_buf,'$');
	 if(cmd_compare_str==NULL){
		 debug_str("\t\tTBR CoM Returning\n");
		 return false;
	 }
	 else{
		 temp_var=parse_message_tbr(cmd_rx_tx_buf);

			debug_str("\t\tTBR CoM Returning\n");

		 if(temp_var>0){
			 return true;
		 }
		 else{
			 return false;
		 }
	 }
		return 0;
}
void clear_buffer(char *buf, uint16_t size){
	int			loop_var=0;
	for(loop_var=0;loop_var<size;loop_var++){
		buf[loop_var]=0;
	}
	return;
}
char 			resuable_buffer[128];
bool get_and_compare(char *compare_string){
	char 			*cmd_compare_str;
	char 			cmd_rx_tx_buf[CMD_RX_TX_BUF_SIZE];
	int				loop_var=0;
	char			temp_char='0';
	bool			ret_flag=false;

	//delay_ms(5);
	debug_str("\tTBR G & C Called\n");////////////////////////////////

	clear_buffer(cmd_rx_tx_buf,CMD_RX_TX_BUF_SIZE);
	delay_ms(9);												//response time from TBR
	//debug_str("TBR RXVD\n");
	for(loop_var=0;loop_var<FIFO_TBR_RX_DATA_SIZE;loop_var++){
		temp_char=rs485_recieve_char();
		if(temp_char=='@'){break;}
		cmd_rx_tx_buf[loop_var]=temp_char;
		//debug_char(temp_char);
		//debug_str("\n");
	}
	////////////////////////////////
	int				debug_var=0;
	/////////////////////////////////
	cmd_compare_str=strstr(cmd_rx_tx_buf,(const char *)compare_string);
	if(cmd_compare_str!=NULL){
		ret_flag=true;
		///////////////////////
		debug_var=loop_var;
		sprintf(resuable_buffer, "\t\tTBR ACK RXD. Buf is:\n");
		debug_str(resuable_buffer);
		for(loop_var=0;loop_var<debug_var;loop_var++){
			sprintf(resuable_buffer, "%c",cmd_rx_tx_buf[loop_var]);
			debug_str(resuable_buffer);
			delay_ms(1);
		}
		sprintf(resuable_buffer, "\n");
		///////////////////////
	}
	 else{
		 ret_flag=false;
////////////////////////////////////////////////////

		debug_var=loop_var;
		sprintf(resuable_buffer, "\t\tTBR NO ack rcvd. Buf is:\n");
		debug_str(resuable_buffer);
		for(loop_var=0;loop_var<debug_var;loop_var++){
			sprintf(resuable_buffer, "%c",cmd_rx_tx_buf[loop_var]);
			debug_str(resuable_buffer);
			delay_ms(1);
		}
		sprintf(resuable_buffer, "\n");
		debug_str(resuable_buffer);
////////////////////////////////////////////////////
	 }
	 check_other_messages(cmd_rx_tx_buf);
	 debug_str("\tTBR G & C Returned\n");/////////////////////
	return ret_flag;
}

uint8_t convert_single_tbr_msg_into_uint(char *single_msg, uint8_t *dst_buf, uint8_t offset){
	//char 			resuable_buffer[128];
	char			*temp_ptr;
	char			ref_token[2]=",";
	char			*token;
	bool			messgae_type=TBR_DETECION_MSG;
	tbr_msesage_t	tbr_message;

	//delay_ms(4);
	//clear_buffer(resuable_buffer,128);
	//sprintf(resuable_buffer, "\t\tSingle:Msg=%s\n",single_msg);
	//debug_str(resuable_buffer);
	//delay_ms(4);
		//$000xxx
	token = strtok(single_msg, ref_token);
		//timestamp
	token = strtok(NULL, ref_token);
	tbr_message.timestamp=(uint32_t)strtoul(token,&temp_ptr,10);
		//TBR Sensor or millisec
	token = strtok(NULL, ref_token);
	if(strncmp(token,(char *)"TBR Sensor",5)==0){
		messgae_type=TBR_SENSOR_MSG;
	}
	else
	{
		tbr_message.millisec=(uint16_t)strtoul(token,&temp_ptr,10);
	}
		//Codetype or Temperature
	token = strtok(NULL, ref_token);
	if(messgae_type==TBR_DETECION_MSG){
		if(strncmp(token,(char *)"S256",4)==0)
			tbr_message.CodeType=00;
		else if(strncmp(token,(char *)"R64K",4)==0)
			tbr_message.CodeType=01;
		else if(strncmp(token,(char *)"R04K",4)==0)
			tbr_message.CodeType=02;
		else if(strncmp(token,(char *)"R256",4)==0)
			tbr_message.CodeType=03;
		else
			tbr_message.CodeType=0xFF;
	}
	else{
		tbr_message.Temperature=(uint16_t)strtoul(token,&temp_ptr,10);
	}
		//CodeID or Noise
	token = strtok(NULL, ref_token);
	if(messgae_type==TBR_DETECION_MSG){
		tbr_message.CodeID=(uint16_t)strtoul(token,&temp_ptr,10);
	}
	else{
		tbr_message.Noise=(uint8_t)strtoul(token,&temp_ptr,10);
	}
		//CodeData or NoiseLP
	token = strtok(NULL, ref_token);
	if(messgae_type==TBR_DETECION_MSG){
		tbr_message.CodeData=(uint16_t)strtoul(token,&temp_ptr,10);
	}
	else{
		tbr_message.NoiseLP=(uint8_t)strtoul(token,&temp_ptr,10);
	}
		//SNR or Frequency
	token = strtok(NULL, ref_token);
	if(messgae_type==TBR_DETECION_MSG){
		tbr_message.SNR=(uint8_t)strtoul(token,&temp_ptr,10);
	}
	else{
		tbr_message.frequency=(uint8_t)strtoul(token,&temp_ptr,10);
	}

	token = strtok(NULL, ref_token);
			//fill the lora buffer
	dst_buf[offset+0]=(uint8_t)(tbr_message.timestamp>>24);
	dst_buf[offset+1]=(uint8_t)(tbr_message.timestamp>>16);
	dst_buf[offset+2]=(uint8_t)(tbr_message.timestamp>>8);
	dst_buf[offset+3]=(uint8_t)(tbr_message.timestamp>>0);
	if(messgae_type==TBR_DETECION_MSG){
		dst_buf[offset+4]=(uint8_t)tbr_message.CodeType;
		dst_buf[offset+5]=(uint8_t)(tbr_message.CodeID>>8);
		dst_buf[offset+6]=(uint8_t)(tbr_message.CodeID>>0);
		dst_buf[offset+7]=(uint8_t)(tbr_message.CodeData>>8);
		dst_buf[offset+8]=(uint8_t)(tbr_message.CodeData>>0);
		uint8_t temp_1=((tbr_message.SNR & 0x3F)<<2);
		uint8_t temp_2=((tbr_message.millisec>>8));
		dst_buf[offset+9]=temp_1 | temp_2;
		dst_buf[offset+10]=(uint8_t)(tbr_message.millisec>>0);
		/*for(int i=0;i<11;i++){
			sprintf(resuable_buffer, "\tSingle:Broken Tag Offset=%d dst_buf[%d]=%2x\n",offset,i,dst_buf[offset+i]);
			debug_str(resuable_buffer);
			delay_ms(4);
		}*/
	}else{
		dst_buf[offset+4]=(uint8_t)0xFF;
		dst_buf[offset+5]=(uint8_t)(tbr_message.Temperature>>8);
		dst_buf[offset+6]=(uint8_t)(tbr_message.Temperature>>0);
		dst_buf[offset+7]=(uint8_t)tbr_message.Noise;
		dst_buf[offset+8]=(uint8_t)tbr_message.NoiseLP;
		dst_buf[offset+9]=(uint8_t)0xFF;
		dst_buf[offset+10]=(uint8_t)tbr_message.frequency;
		/*for(int i=0;i<11;i++){
			sprintf(resuable_buffer, "\tSingle:Broken Sensor Offset=%d  dst_buf[%d]=%2x\n",offset,i,dst_buf[offset+i]);
			debug_str(resuable_buffer);
			delay_ms(4);
		}*/
	}
	return offset+11;		//fixed offset=message size - 1.....*/
}
uint8_t convert_tbr_msgs_to_uint(char *src_buf, uint8_t *dst_buf, uint8_t msg_count){
	uint16_t 		inner_loop_var=0;
	uint16_t		outer_loop_var=0;
	uint16_t		offset_src_buf=0;
	uint16_t		offset_dst_buf=0;
	char 			single_msg[50];
	char			*temp_ptr;
	uint16_t		messages_converted=0;

	clear_buffer(single_msg, 50);
		//extract and convert SN to uint8_t
	for(outer_loop_var=0;outer_loop_var<10;outer_loop_var++){
		if(src_buf[outer_loop_var+1]==','){break;}
		single_msg[outer_loop_var]=src_buf[outer_loop_var+1];	//+1 to ignore $
	}
	dst_buf[0]=(uint8_t)strtoul(single_msg,&temp_ptr,10);
	dst_buf[0]=0x84;
	offset_dst_buf=1;
		//now convert rest of the messages into uint8_t (7 bytes per message => TimeStamp(4)+milli_sec(2)+tagID(1))
	offset_src_buf=0;
	if(msg_count>10){
		sprintf(resuable_buffer, "\tMessage length =%d. NO LoRA parse\n",msg_count);
		debug_str(resuable_buffer);
		return 0;
	}
	for(outer_loop_var=0;outer_loop_var<msg_count;outer_loop_var++){
		clear_buffer(single_msg, 50);
		for(inner_loop_var=0;inner_loop_var<strlen(src_buf);inner_loop_var++){
			if(src_buf[offset_src_buf+inner_loop_var]=='\n'){
				offset_src_buf+=inner_loop_var+1;
				//sprintf(resuable_buffer, "\tSingle: Offset Src buffer=%d\n",offset_src_buf);
				//debug_str(resuable_buffer);
				break;
			}
			single_msg[inner_loop_var]=src_buf[offset_src_buf+inner_loop_var];
		}

//		if((strstr(single_msg,(const char*)"TBR Sensor")==NULL) && (strstr(single_msg,(const char*)"ack")==NULL)){		//only add detections NOT sensor values....
		if((strstr(single_msg,(const char*)"ack")==NULL) && strlen(single_msg)>30){
			offset_dst_buf=convert_single_tbr_msg_into_uint(single_msg,dst_buf,offset_dst_buf);
			messages_converted++;
		}
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

	//delay_ms(5);
	//debug_str("\tTBR Called\n");

	if( tbr_cmd==cmd_sn_req){
		sprintf((char *)cmd_tx_buf,"?\n");
		rs485_transmit_string(cmd_tx_buf,1);
		ret_flag=get_and_compare((char *)"SN=");
	}
	else if(tbr_cmd==cmd_basic_sync){
		sprintf((char *)cmd_tx_buf,"(+)\n");
		rs485_transmit_string(cmd_tx_buf,3);
		ret_flag=get_and_compare((char *)"ack01\r");			//changed from 01
	}
	else if(tbr_cmd==cmd_advance_sync){
		my_timestamp=timestamp;
		luhn=CalculateLuhn(&my_timestamp);
		sprintf((char *)cmd_tx_buf,"(+)%ld\n",my_timestamp);
		temp_var=strlen((const char *)cmd_tx_buf);
		cmd_tx_buf[temp_var-2]=luhn;						//change last digit of TimeStamp
		rs485_transmit_string(cmd_tx_buf,temp_var-1);
		ret_flag=get_and_compare((char *)"ack01\rack02\r");		//changed from 02
	}
	else{
		ret_flag=false;
	}
	/*debug_str("\tTBR Returning\n");
	for(int i=0;i<10;i++){
		debug_str("\tHello shit I am working\n");
	}*/
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

uint8_t tbr_recv_msg_uint(uint8_t *lora_msg_buf, int *lora_length, char *msg_buf, int *msg_length)
{
	int 			loop_var=0;
	int 			msg_count=0;
	uint8_t			lora_buf_length=0;
	char			temp_char='0';
	char			intermediate_buffer[ARRAY_MESSAGE_SIZE];

		//SD card msg_buffer
	clear_buffer(msg_buf, ARRAY_MESSAGE_SIZE);
	clear_buffer(intermediate_buffer, ARRAY_MESSAGE_SIZE);

	sprintf(resuable_buffer, "\t\t\tMSg buffer is:\n");
	debug_str(resuable_buffer);

	while(!array_is_empty()){
		temp_char=array_remove();

		sprintf(resuable_buffer, "%c",temp_char);
		debug_str(resuable_buffer);

		if(temp_char=='$'){
			msg_count++;
		}
		msg_buf[loop_var]=temp_char;
		loop_var++;
	}
	sprintf(resuable_buffer, "\n");
	debug_str(resuable_buffer);

	*msg_length=loop_var;
		//LoRa buffer
	lora_buf_length=convert_tbr_msgs_to_uint(msg_buf,lora_msg_buf,(uint8_t)msg_count);
	lora_buf_length=((lora_buf_length*11)+1);
	if(lora_buf_length<=2){lora_buf_length=0;}	//exclude Serial Number byte....
	*lora_length=lora_buf_length;
	return msg_count;
}
