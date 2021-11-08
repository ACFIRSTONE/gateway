
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
#include "debug_bsp.h"
/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/


/*
*********************************************************************************************************
Typedef
*********************************************************************************************************
*/
typedef void (*interrupt_callback)(void);

/*
*********************************************************************************************************
Variables
*********************************************************************************************************
*/
extern key_value_frame_t user_key_data;
extern key_value_frame_t user_key_analysis;
extern key_value_frame_t user_key_execute;
extern system_information_t user_system_data;
extern key_value_delay_frame_t  user_delay_key;
extern user_logic_t user_logic;
extern user_total_information_t  current_user;
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
/**
*********************************************************************************************************
* @����	: 
* @����	:����û���ֵ����
*********************************************************************************************************
**/
void key_buff_clear(void)
{
	memset(&user_key_data, 0, sizeof( key_value_frame_t ));
	user_key_data.key_opint=0;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ֵ���򻺴�
*********************************************************************************************************
**/
void key_write_directional_cache(u8 *buff , u8 type , key_value_frame_t *target_cache)
{
	if(target_cache->key_opint < KEY_VALUE_NUM)
	{
		/*��1��ʼ�洢*/
		target_cache->key_opint++;
		/*���ݻ���*/
		memcpy(&target_cache->keyvalue[target_cache->key_opint], buff, sizeof(target_cache->keyvalue[0].buff));
		target_cache->keyvalue[target_cache->key_opint].type = type;  /*  ����Ϊ��� �ڲ��ⲿת������  */
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�û���ֵ����д��
*********************************************************************************************************
**/
void key_buff_write(u8* buff,u8 type)
{	
	key_write_directional_cache(buff , type , &user_key_analysis);
	
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ֵ�������
*********************************************************************************************************
**/
u8 key_read_directional_cache(u8 *buff , key_value_frame_t *target_cache)
{
	u8 ret_data = 0;
	if(target_cache->key_opint > 0)
	{
		/*���ƻ���*/
		memcpy(buff, &target_cache->keyvalue[target_cache->key_opint],sizeof(target_cache->keyvalue[0].buff));
		ret_data = target_cache->keyvalue[target_cache->key_opint].type;
		/*��ջ���*/
		memset(&target_cache->keyvalue[target_cache->key_opint], 0, sizeof(target_cache->keyvalue[0].buff));
		target_cache->key_opint--;		
	}
	return ret_data;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�û���ֵ����ִ��
*********************************************************************************************************
**/
u8 key_buff_read(u8* buff)
{
	u8 ret_data = 0;
	ret_data = key_read_directional_cache( buff ,&user_key_execute);
	return ret_data;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ֵ����
*********************************************************************************************************
**/
void key_delay_buff_write(u8* buff, u8 type, u32 delay)
{
	for(u8 i = 0 ; i <KEY_VALUE_NUM ; i++)
	{
		if(user_delay_key.keyvalue[i].flag == 0)
		{
			memcpy(&user_delay_key.keyvalue[i], buff, sizeof(user_delay_key.keyvalue[0].buff));
			user_delay_key.keyvalue[i].type = type;  /*  ����Ϊ��� �ڲ��ⲿת������  */
			user_delay_key.keyvalue[i].delay = delay;
			user_delay_key.keyvalue[i].flag = 1;
			break;
		}
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ֵ����
*********************************************************************************************************
**/
void key_delay_buff_directional_clear(u8 value)
{
	for(u8 i = 0 ; i <KEY_VALUE_NUM ; i++)
	{
		if(user_delay_key.keyvalue[i].flag != 0)  
		{
			if(user_delay_key.keyvalue[i].buff[2] == value)
			{
				/*������л�������*/
				memset( &user_delay_key.keyvalue[i], 0, sizeof(key_value_delay_t) );
			}				
		}
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ֵɨ�裬�����ִ�л���
*********************************************************************************************************
**/
void key_delay_scanf_handle(void)
{
	/*10MSִ��һ��*/
	for(u8 i = 0 ; i <KEY_VALUE_NUM ; i++)
	{
		if(user_delay_key.keyvalue[i].flag != 0)
		{
			if(user_delay_key.keyvalue[i].delay > 0)
			{
				user_delay_key.keyvalue[i].delay--;	
			}
			else
			{
				key_write_directional_cache(user_delay_key.keyvalue[i].buff , user_delay_key.keyvalue[i].type , &user_key_execute);
				/*������л�������*/
				memset( &user_delay_key.keyvalue[i], 0, sizeof(key_value_delay_t) );					
			}
		}
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�ص������豸 �ϵ��ǿ���ʹ�� 
*********************************************************************************************************
**/
void key_card_power_off(void)
{
	u8 key_buff[5];
	/*�忨*/
	key_buff[STATUS_BYTE] =  0;
	key_buff[DATA_BYTE] = 193; 
	key_write_directional_cache(key_buff, 0, &user_key_execute);
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ֵ���涨���ж�
*********************************************************************************************************
**/
u8 key_directional_check(key_value_frame_t *target_cache)
{	
	if(target_cache->key_opint > 0)
		return 1;
	return 0;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ֵ�����ж� 
*********************************************************************************************************
**/
u8 key_buff_check(void)
{	
	if(key_directional_check(&user_key_execute) == 1)
		return 1;
	return 0;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�忨�ж�
*********************************************************************************************************
**/
void key_card_power_on_judge(key_value_t *key_logic)
{
	u8 key_buff[5];
	/*�忨�жϲ���������1 2*/
	if(key_logic->buff[DATA_BYTE] == 0x14)
	{
		if(key_logic->buff[STATUS_BYTE] == 1)
		{
			/*�忨*/
			key_buff[STATUS_BYTE] =  0;
			key_buff[DATA_BYTE] = 192; 
			key_write_directional_cache(key_buff, 0, &user_key_execute);
			/*����ο���ʱ*/
			key_delay_buff_directional_clear(193);
			user_logic.power_on_card = 1;
		}		
		else if(key_logic->buff[STATUS_BYTE] == 2)
		{
				/*�ο�*/
			key_buff[STATUS_BYTE] =  0;			
			key_buff[DATA_BYTE] = 193;
			
			if(user_system_data.set.pick_up_card_delay[1] > 60)
				key_delay_buff_write(key_buff, 0, 6000);
			else
				key_delay_buff_write(key_buff, 0, user_system_data.set.pick_up_card_delay[1]*100);	
			
			user_logic.power_on_card = 0;
		}
	}	
	/*  �ж�ӳ������е� ���ܲ忨���Ƶ�ָ��*/	
	if(user_logic.power_on_card == 0)
	{
		if(key_logic->buff[DATA_BYTE] != 0x14)
		{
			u8 limit = 0;
			for(u8 i = 0 ; i < 10 ; i++)
			{
				if(user_system_data.set.user_mapping_set[i].limit == 1)
				{
					if(user_system_data.set.user_mapping_set[i].function ==	 key_logic->buff[DATA_BYTE])
					{
						limit = 1;
						break;
					}			
				}		
			}
			if(limit == 0)
				key_logic->buff[DATA_BYTE]	= 0xff; 
		}			
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�����ж�  
*********************************************************************************************************
**/
void key_mutex_judge(key_value_t *key_logic)
{
	u8 key_buff[5];
	/*����״̬�޷�����*/
	if(current_user.service.do_not_disturb_status == 1)
	{
		if(key_logic->buff[DATA_BYTE] == 22)
		{
			key_logic->buff[DATA_BYTE]	= 0xff; 
		}
	}
	/*���ż�ֵ�������״̬*/	
	if(key_logic->buff[DATA_BYTE] == 24)
	{

		if(current_user.service.cleaning_status == 1)	
		{
			current_user.service.cleaning_status = 0;
			key_buff[STATUS_BYTE] =  2;				
			key_buff[DATA_BYTE] = 22;
			key_write_directional_cache(key_buff, 0, &user_key_execute);
		}
	}	
}
/**
*********************************************************************************************************
* @����	: 
* @����	:ӳ���ж�  ԭ��ֵ����  ����һ���ƿؼ�ֵ�� ��ʱ���Ƽ�ֵ  
*********************************************************************************************************
**/
void key_mapping_judge(key_value_t *key_logic)
{
	u8 key_buff[5];
	for(u8 i = 0 ; i < 10 ; i++)
	{
		if(user_system_data.set.user_mapping_set[i].channel <= 40)
		{
			/*ƥ���ֵ*/
			if((user_system_data.set.user_mapping_set[i].function == key_logic->buff[DATA_BYTE])  && (key_logic->buff[DATA_BYTE] != 255))
			{
				/*�ж�����*/
				if(user_system_data.set.user_mapping_set[i].trigger == 0) /*��ͨ����*/
				{
					/*ֱ����ʱ����*/
					key_buff[STATUS_BYTE] =  0;			
					key_buff[DATA_BYTE] = KEY_VALUE_LAMP_START - 1 + user_system_data.set.user_mapping_set[i].channel;	
					key_delay_buff_write(key_buff, 0, ((u16)(user_system_data.set.user_mapping_set[i].delay[0]<<8) + user_system_data.set.user_mapping_set[i].delay[1])*100);				
				}	
				else if(user_system_data.set.user_mapping_set[i].trigger == 2)/*��ʱ��������   ��ʱһ��ʱ���*/
				{
					/*��ʱ���Ϳ�*/
					key_buff[STATUS_BYTE] =  1;			
					key_buff[DATA_BYTE] = KEY_VALUE_LAMP_START- 1 + user_system_data.set.user_mapping_set[i].channel;	
					key_delay_buff_write(key_buff, 0, ((u16)(user_system_data.set.user_mapping_set[i].delay[0]<<8) + user_system_data.set.user_mapping_set[i].delay[1])*100);					
				}				
				break;
			}			
		}		
	}		
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ֵ�߼������忨�жϣ������жϣ���ӳ���ж�
*********************************************************************************************************
**/
void key_value_logic_handle(void)
{
	key_value_t  key_logic;
	if(key_directional_check(&user_key_analysis) == 1)
	{
		/*��ȡ��ֵ*/
		key_logic.type = key_read_directional_cache(key_logic.buff ,&user_key_analysis);	
		/*�忨�ж�*/
		key_card_power_on_judge(&key_logic);
		/*�����ж�*/
		key_mutex_judge(&key_logic);		
		/*ӳ���ж�*/
		key_mapping_judge(&key_logic);
		/*д�뻺��*/	
		if(key_logic.buff[DATA_BYTE]	!= 0xff)
			key_write_directional_cache(key_logic.buff , key_logic.type , &user_key_execute);
	}
}
/***********************************************END*****************************************************/

