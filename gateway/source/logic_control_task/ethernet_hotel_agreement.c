
/**
*********************************************************************************************************
*                                        		gateway
*                                      (c) Copyright 2021-2031
*                                         All Rights Reserved
*
* @File    : 
* @By      : liwei
* @Version : V0.02
* 
*********************************************************************************************************
**/

/*
*********************************************************************************************************
Includes 
*********************************************************************************************************
*/
#include "hc32f46x_clk.h"
#include "FreeRTOS.h"
#include "task.h"
#include "logic_control_task.h"
#include "broker_task.h"
#include "user_type.h"
#include "data_task.h"
/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
#define SEND_NUM 120

/*
*********************************************************************************************************
Typedef
*********************************************************************************************************
*/
typedef u8 (*callback_t)(u8* buff , data_container_t* current_data_container);

/*
*********************************************************************************************************
Variables
*********************************************************************************************************
*/
extern data_container_t  current_data_container;
extern system_information_t user_system_data;
extern user_total_information_t  current_user;
extern release_buff_t logic_business_release_buff;
extern  u8 key_value_updata_flag;
extern user_logic_t user_logic;

u8 ethernet_debug_flag = 0;
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
u8  get_cloud_connect_state(void);
void hotel_air_conditioner_package_send(air_conditioner_t *air_conditioner );
void null(u8 *buff , data_container_t* current_data_container){}
	
/**
*********************************************************************************************************
* @����	: 
* @����	: ID ת�� 10λһ����  3��ID      30BIT  ȥ��2BIT 
*********************************************************************************************************
**/
void id_transformation(u8 *buff, u8 *id_buff)
{
	buff[0]= (id_buff[0]*256+id_buff[1]) ;      	 	//��8λ
	buff[1]= ((id_buff[0]*256+id_buff[1])>>8)&0x03; 	//��2λ
	
	buff[1]= ((id_buff[2]*256+id_buff[3]))<<2;
	buff[2]= ((id_buff[2]*256+id_buff[3]))>>4;
	
	buff[2]= ((id_buff[4]*256+id_buff[5]))<<4;
	buff[3]= ((id_buff[4]*256+id_buff[5]))>>4;		
}
/**
*********************************************************************************************************
* @����	: 
* @����	:������������
*********************************************************************************************************
**/
void ethernet_send_hearbeat(data_container_t* current_data_container)
{
/*�ؼ����ֲ��㹫����ɾ������*/
}
/**
*********************************************************************************************************
* @����	: 
* @����	: 00 00 00 08 00 01 01 01    ��00 00 00 08 00 01 01 00
*********************************************************************************************************
**/
u8 hotel_lamp_instructions(u8 *buff , data_container_t* current_data_container)
{
	if(buff[6] < 40)
	{		
		u8 key_buff[3];
		key_buff[0] = 0;
		if(buff[7] == 0)
			key_buff[STATUS_BYTE] = 2; 
		else
			key_buff[STATUS_BYTE] = 1;
		key_buff[DATA_BYTE] = buff[6]  + KEY_VALUE_LAMP_START; 
		key_buff_write(key_buff, 0);/*��Ǽ�ֵ����Ϊ�ڲ�ת��*/	
	}
	return 8;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
u8 hotel_scene_instructions(u8 *buff , data_container_t* current_data_container)
{
	u8 key_buff[3];
	key_buff[0] = 0;	
	key_buff[STATUS_BYTE] = 0; 
	key_buff[DATA_BYTE] = KEY_VALUE_SECEN_START+buff[6]; 
	/*����ģʽ��ֵ*/
	key_buff_write(key_buff, 0);
	return 7;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
u8 hotel_combination_instructions(u8 *buff , data_container_t* current_data_container)
{
	u8 key_buff[3];
	key_buff[0] = 0;	
	key_buff[STATUS_BYTE] = 0; 
	key_buff[DATA_BYTE] = KEY_VALUE_COMBINATION_START+buff[6]; 
	/*������ϼ�ֵ*/
	key_buff_write(key_buff, 0);
	return 7;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
u8 hotel_card_power_on_instructions(u8 *buff , data_container_t* current_data_container)
{
	u8 key_buff[3];
	key_buff[0] = 0;	
	key_buff[STATUS_BYTE] = buff[6]; 
	key_buff[DATA_BYTE] = 0x14; 
	/*20�忨 �ο�����*/
	key_buff_write(key_buff, 0);
	return 7;	
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
u8 hotel_doorbell_instructions(u8 *buff , data_container_t* current_data_container)
{
	return 7;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/	
u8 hotel_do_not_disturb_instructions(u8 *buff , data_container_t* current_data_container)
{	
	u8 key_buff[3];
	key_buff[0] = 0;	
	key_buff[STATUS_BYTE] = buff[6]; 
	key_buff[DATA_BYTE] = 0x18; 
	key_buff_write(key_buff, 0);/*��Ǽ�ֵ����Ϊ�ڲ�ת��*/	
	return 7;	
}
/**
*********************************************************************************************************
* @����	: 
* @����	: 
*********************************************************************************************************
**/	
u8 hotel_cleaning_instructions(u8 *buff , data_container_t* current_data_container)
{
	u8 key_buff[3];
	key_buff[0] = 0;	
	key_buff[STATUS_BYTE] = buff[6]; 
	key_buff[DATA_BYTE] = 0x16; 
	key_buff_write(key_buff, 0);/*��Ǽ�ֵ����Ϊ�ڲ�ת��*/
	return 7;	
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
u8 hotel_sos_instructions(u8 *buff , data_container_t* current_data_container)
{
	u8 key_buff[3];
	key_buff[0] = 0;	
	key_buff[STATUS_BYTE] = buff[6]; 
	key_buff[DATA_BYTE] = 21; 
	key_buff_write(key_buff, 0);
	return 7;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/	
u8 hotel_call_service_instructions(u8 *buff , data_container_t* current_data_container)
{
	u8 key_buff[3];
	key_buff[0] = 0;	
	key_buff[STATUS_BYTE] = buff[6]; 
	key_buff[DATA_BYTE] = 0x1B; 
	key_buff_write(key_buff, 0);/*��Ǽ�ֵ����Ϊ�ڲ�ת��*/
	return 7;	
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
u8 hotel_reservation_check_instructions(u8 *buff , data_container_t* current_data_container)
{	
	u8 key_buff[3];
	key_buff[0] = 0;	
	key_buff[STATUS_BYTE] = buff[6]; 
	key_buff[DATA_BYTE] = 0x17; 
	key_buff_write(key_buff, 0);/*��Ǽ�ֵ����Ϊ�ڲ�ת��*/	
	return 7;	
}

/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
u8 hotel_air_conditioner_instructions(u8 *buff , data_container_t* current_data_container)
{	
	/*���¿յ���Ϣ*/	
	current_user.air_conditioner[0].set_temperature =  	buff[7];
	current_user.air_conditioner[0].wind_speed =  		buff[8];
	current_user.air_conditioner[0].wind_direction =  	buff[9];
	current_user.air_conditioner[0].power =  			buff[10];
	current_user.air_conditioner[0].null =  			buff[11];
	current_user.air_conditioner[0].mode =  			buff[12];
	key_value_updata_flag = 1;
	hotel_air_conditioner_package_send(&current_user.air_conditioner[0]);
	return 13;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
u8 hotel_music_add_sub_instructions(u8 *buff , data_container_t* current_data_container)
{	
	u8 key_buff[3]={0,0,0};
	/*��ַ*/
	if(buff[6] == 0)
		key_buff[DATA_BYTE] = 170; 	
	else if(buff[6] == 1)
		key_buff[DATA_BYTE] = 171; 
	
	key_buff_write(key_buff, 0);	
	return 7;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
u8 hotel_music_up_down_instructions(u8 *buff , data_container_t* current_data_container)
{	
	u8 key_buff[3]={0,0,0};
	/*��ַ*/
	if(buff[6] == 0)
		key_buff[DATA_BYTE] = 172; 		
	else if(buff[6] == 1)
		key_buff[DATA_BYTE] = 173; 

	key_buff_write(key_buff, 0);	
	return 7;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
u8 hotel_music_play_instructions(u8 *buff , data_container_t* current_data_container)
{	
	u8 key_buff[3]={0,0,0};
	/*��ַ*/
	if(buff[6] == 0)
		key_buff[DATA_BYTE] = 175; 		
	else if(buff[6] == 1)
		key_buff[DATA_BYTE] = 177; 
	else if(buff[6] == 2)
		key_buff[DATA_BYTE] = 176;
	key_buff_write(key_buff, 0);	
	return 7;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
u8 hotel_curtain_instructions(u8 *buff , data_container_t* current_data_container)
{	
	u8 key_buff[3]={0,0,0};
	/*��ַ*/
	if(buff[6] == 0)
	{
		if(buff[7] == 1)
			key_buff[DATA_BYTE] = 162; 
		else if(buff[7] == 2)
			key_buff[DATA_BYTE] = 163; 
		key_buff_write(key_buff, 0);		
	}
	else if(buff[6] == 1)
	{	 
		if(buff[7] == 1)
			key_buff[DATA_BYTE] = 165; 
		else if(buff[7] == 2)
			key_buff[DATA_BYTE] = 166; 
		key_buff_write(key_buff, 0);	
	}
	return 7;	
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
u8 hotel_open_door_instructions(u8 *buff , data_container_t* current_data_container)
{	
	u8 key_buff[3]={0,0,0};
	/*��ַ*/
	if(buff[6] == 0)
	{
		key_buff[STATUS_BYTE] = 2; 
		key_buff[DATA_BYTE] = 30; 
		key_buff_write(key_buff, 0);			
	}
	else if(buff[6] == 1)
	{
		key_buff[STATUS_BYTE] = 1; 		
		key_buff[DATA_BYTE] = 30; 
		key_buff_write(key_buff, 0);
		
	}
	return 7;		
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
#define HOTEL_CALLBACK_MAX 19
u8 hotel_null_instructions(u8 *buff , data_container_t* current_data_container)
{
	return 0;
}
const callback_t hotel_instruction_pack[HOTEL_CALLBACK_MAX] = 
{
	hotel_lamp_instructions,
	hotel_scene_instructions,
	hotel_combination_instructions,
	hotel_card_power_on_instructions,
	hotel_doorbell_instructions,
	hotel_do_not_disturb_instructions,
	hotel_cleaning_instructions,
	hotel_sos_instructions,
	hotel_call_service_instructions,	
	hotel_reservation_check_instructions,
	hotel_air_conditioner_instructions,
	hotel_null_instructions,
	hotel_null_instructions,
	hotel_null_instructions,
	hotel_null_instructions,
	hotel_null_instructions,
	hotel_null_instructions,
	hotel_curtain_instructions,
	hotel_open_door_instructions,
};
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
void ethernet_data_analysis(data_container_t* data_container ,data_container_t* current_data_container)
{
	if( ( strstr("ethernet"  ,(char*)data_container->name ) ) != NULL )  /*�жϷ������Ƿ�Ϊethernet*/
	{
		u16 ins_surplus_length = data_container->length;
		u16 ins_current_length = 7;
		while(ins_surplus_length > 7)
		{
			if((data_container->buff[0] == 0 ) && (data_container->buff[1] == 0 ))
			{
				/*ִ��ָ��ص�����*/
				if((data_container->buff[5] > 0) && (data_container->buff[5] <= HOTEL_CALLBACK_MAX))
					ins_current_length = hotel_instruction_pack[data_container->buff[5] - 1](data_container->buff , current_data_container);	

				if(ins_surplus_length > ins_current_length)
				{
					ins_surplus_length = ins_surplus_length - ins_current_length;
					memcpy(&data_container->buff[0] , &data_container->buff[ins_current_length] , ins_surplus_length);
				}
				else
					ins_surplus_length = 0;
			}
			else
				break;
		
		}		
	}
	else if( ( strstr("local_ethernet"  ,(char*)data_container->name ) ) != NULL )  /*�жϷ������Ƿ�Ϊlocal_ethernet*/
	{
		/*�ؼ����ֲ��㹫����ɾ������*/				
	}		
}
/***********************************************END*****************************************************/

