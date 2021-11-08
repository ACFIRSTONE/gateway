
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
#define PROGRAM_FIXED_DELAY 5
#define FORMAT_MS 0
#define FORMAT_S 1

/*
*********************************************************************************************************
Typedef
*********************************************************************************************************
*/


/*
*********************************************************************************************************
Variables
*********************************************************************************************************
*/
static data_container_t  current_data_container;
extern system_information_t user_system_data;
extern user_total_information_t  current_user;
extern user_logic_t user_logic;
extern release_buff_t logic_business_release_buff;
extern  u8 key_value_updata_flag;
extern u8 ethernet_debug_flag;
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
void hotle_agreement_handle(u8* key_buff, data_container_t  *current_data_container);
void inside_lookup_handle(u8* key_buff,u8 type, data_container_t  *current_data_container);	
void outside_lookup_handle(u8* key_buff,u8 type, data_container_t  *current_data_container);
static void debug_ethernet_transmission(u8* buff, u8 length , u8 port ,u8 cmd);
static void transparent_ethernet_transmission(u8* buff, u8 length );
/**
*********************************************************************************************************
* @����	: 
* @����	:�Ƶ�Э�����
*********************************************************************************************************
**/
static void hotle_agreement_analysis(data_container_t* subscribe)
{
	if((subscribe->buff[0] == 0xf2) && (subscribe->length == 4))
	{
		/*����3�ֽڼ�ֵ*/
		key_buff_write(&subscribe->buff[1],0);	
	}		
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
static void inside_lookup_analysis(data_container_t* subscribe)
{
	/*���β��*/	
	for(u8 i=0; i < MACTH_TABLE_NUM; i++)
	{
		/*�жϳ��Ȳ�Ϊ0*/
		if((subscribe->length != 0)&&(user_system_data.logic.inside_table[i].length != 0))
		{
			if(compare_hex_buff(subscribe->buff, user_system_data.logic.inside_table[i].buff , 
				(subscribe->length > user_system_data.logic.inside_table[i].length) ? subscribe->length :user_system_data.logic.inside_table[i].length))/* ����ʵ�ʳ��ȱȽ� ע����ֵ���*/
			{
				/*����3�ֽڼ�ֵ*/		
				u8 key_buff[3];
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 0 ; 
				key_buff[DATA_BYTE] =user_system_data.logic.inside_table[i].value; 
				key_buff_write(key_buff, 1);/*��Ǽ�ֵ����Ϊ�ڲ�ת��*/
				break;
			}
		}
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
static void outside_lookup_analysis(data_container_t* subscribe )
{
	/*���β��*/
	for(u8 i=0; i < MACTH_TABLE_NUM; i++)
	{
		/*�жϳ��Ȳ�Ϊ0*/
		if((subscribe->length != 0)&&(user_system_data.logic.outside_table[i].length != 0))
		{
			if(compare_hex_buff(subscribe->buff, user_system_data.logic.outside_table[i].buff , 
				(subscribe->length > user_system_data.logic.outside_table[i].length) ? subscribe->length :user_system_data.logic.outside_table[i].length))/* ����ʵ�ʳ��ȱȽ� ע����ֵ���*/		
			{
				/*����3�ֽڼ�ֵ*/		
				u8 key_buff[3];
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 0 ; 
				key_buff[DATA_BYTE] =user_system_data.logic.outside_table[i].value; 
				key_buff_write(key_buff, 2);/*��Ǽ�ֵ����Ϊ�ⲿת��*/
				break;
			}
		}
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�Ƚ�����HEX  BUFF�Ƿ�һ��
*********************************************************************************************************
**/
u8 compare_hex_buff(u8* buff_s, u8* buff_d , u8 length)
{
	for(u8 i=0; i < length; i++)
		if(buff_s[i] != buff_d[i])
			return 0;
	return 1;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�ڲ�RS485���ݽ��� 
*********************************************************************************************************
**/
static void inside_485_analysis(data_container_t* data_container)
{
	if( ( strstr("rs485inside"  , (char*)data_container->name ) ) != NULL )  /*�жϷ������Ƿ�Ϊrs485inside*/
	{
		/*�Ƶ�Э��������� �ڲ���Ҫ�жϷ���*/
		hotle_agreement_analysis( data_container );
		/*���Э���������*/
		inside_lookup_analysis( data_container );
		/*��������*/	
		if( 1 == ethernet_debug_flag )	
			debug_ethernet_transmission(data_container->buff , data_container->length , 0x71 , 0x7b);
		/*͸����������*/
		if((data_container->buff[0] == 0x7a)&&(data_container->buff[1] == 0x71))
		{
			transparent_ethernet_transmission(data_container->buff , data_container->length );
		}
	}
}

/**
*********************************************************************************************************
* @����	: 
* @����	:������Ϣ
*********************************************************************************************************
**/
static void debug_ethernet_transmission(u8* buff, u8 length , u8 port ,u8 cmd)
{	
	/*͸������*/	
	memcpy(current_data_container.name, "logic_ethernet_local", sizeof("logic_ethernet_local"));
	current_data_container.buff[0] = cmd;
	current_data_container.buff[1] = port;
	memcpy(&current_data_container.buff[2], buff, length);
	current_data_container.serial = 1;
	current_data_container.length = 2 + length;
	
	release_buff_write( &logic_business_release_buff, &current_data_container, 5); //��ʱ��������	
	
}
/**
*********************************************************************************************************
* @����	: 
* @����	:͸����Ϣ
*********************************************************************************************************
**/
static void transparent_ethernet_transmission(u8* buff, u8 length)
{	
	/*͸������*/	
	memcpy(current_data_container.name, "logic_ethernet_local", sizeof("logic_ethernet_local"));

	current_data_container.length = length;
	memcpy(&current_data_container.buff, buff, length);
	current_data_container.serial = 1;	
	
	release_buff_write( &logic_business_release_buff, &current_data_container, 5); //��ʱ��������	
	
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�ⲿRS485���ݽ���
*********************************************************************************************************
**/
static void outside_485_analysis(data_container_t* data_container)
{
	if( ( strstr("rs485outside"  ,(char*)data_container->name ) ) != NULL )  /*�жϷ������Ƿ�Ϊrs485inside*/
	{
		/*���Э���������*/
		outside_lookup_analysis( data_container );
		/*��������*/	
		if( 1 == ethernet_debug_flag )	
			debug_ethernet_transmission(data_container->buff , data_container->length , 0x72, 0x7b);
		if((data_container->buff[0] == 0x7a)&&(data_container->buff[1] == 0x72))
		{
			transparent_ethernet_transmission(data_container->buff , data_container->length);
		}		
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:RS485���ݽ���
*********************************************************************************************************
**/
void communication_485_analysis(data_container_t* data_container)
{
	/*�ڲ���������*/
	inside_485_analysis(data_container);	
	/*�ⲿ��������*/
	outside_485_analysis(data_container);
		
}
/**
*********************************************************************************************************
* @����	: 
* @����	:ͨѶ���ݴ��� ����current_user�е�����  
*********************************************************************************************************
**/
void communication_485_handle(u8* key_buff ,u8 type,data_container_t  *current_data_container)
{
	/*�ⲿЭ���ֵ����*/	
	outside_lookup_handle(key_buff , type, current_data_container);	
	/*�ڲ�Э���ֵ����*/
	inside_lookup_handle(key_buff , type, current_data_container);
	/*�Ƶ�Э���ֵ����*/
	hotle_agreement_handle(key_buff, current_data_container);
}
/**
*********************************************************************************************************
* @����	: 
* @����	:40����״̬ ӳ�䵽5�ֽ�
*********************************************************************************************************
**/
void lamp_byte_mapping(u8 *lamp_byte_buff, u8 *lamp_data_buff)
{
	/*40�ƶ�Ӧ5�ֽ�*/
	for(u8 i = 0; i < 5; i++)
	{
		/*����*/
		lamp_byte_buff[ i ] = 0;
		for(u8 j = 0; j < 8 ; j++)
		{
			/*����8BIT����*/
			if(lamp_data_buff[i*8 + j] != 0)
				lamp_byte_buff[ i ] |= 0x01<<j;
			else
				lamp_byte_buff[ i ] &= ~(0x01<<j);		
		}
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:16��� ӳ�䵽2�ֽ�   cmd 0 ȡ�� 1��  2 �� 
*********************************************************************************************************
**/
void combined_byte_mapping(lamp_status_t* lamp_buff, u8 combined , u8 cmd)
{
	if(cmd == 1) /*��1*/
	{
		if(combined < 8)
			lamp_buff->combination[0] |= 0x01<<	combined;
		else if(combined < 16)
			lamp_buff->combination[1] |= 0x01<<	(combined-8);
	}
	else if(cmd == 2)/*��0*/
	{
		if(combined < 8)
			lamp_buff->combination[0] &= ~(0x01<<combined);
		else if(combined < 16)
			lamp_buff->combination[1] &= ~(0x01<<(combined-8));	
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ָ�� ���  agreement_version=0 51�ֽ�  agreement_version = 1  50�ֽ� 
*********************************************************************************************************
**/
void lamp_f1_command_package(u8* package_buff, lamp_status_t* lamp_buff , u16 delay ,u8 second_flag , u8 agreement_version)
{
/*�ؼ����ֲ��㹫����ɾ������*/
}
/**
*********************************************************************************************************
* @����	: 
* @����	:���Ƴ���ģʽ��40����״̬ 
*********************************************************************************************************
**/
u16 scene_mode_lamp_update_status(lamp_status_t* lamp_buff ,scene_buff_t *mode_buff)
{
	for(u8 i = 0; i < 40 ; i ++)
		if( mode_buff->data[ 11 + i ] != 0xff) /*������nc״̬*/
			lamp_buff->lamp_buff[ i ] =	 mode_buff->data[ 11 + i ];
	/*��ʱ*/				
	return ((u16)mode_buff->data[ 9 ]*256 + mode_buff->data[ 10 ]);
}
/**
*********************************************************************************************************
* @����	: 
* @����	:���ݲ���ģʽ������ϵ�״̬ 
*********************************************************************************************************
**/
u16 combination_lamp_update_status(lamp_status_t* lamp_buff ,scene_buff_t *mode_buff , u8 cmd)
{
	/*״̬Ϊ1 �����������ִ�У�״̬Ϊ0 �����������ȡ��ִ�� */
	if(cmd == 1) /*�����������*/
	{
		for(u8 i = 0; i < 40 ; i ++)
			if( mode_buff->data[ 11 + i ] != 0xff) /*������nc״̬*/
				lamp_buff->lamp_buff[ i ] =	 mode_buff->data[ 11 + i ];
	}	
	/*��ʱ*/				
	return ((u16)mode_buff->data[ 9 ]*256 + mode_buff->data[ 10 ]);	
}
/**
*********************************************************************************************************
* @����	: 
* @����	:ͬ����ϵ�״̬
*********************************************************************************************************
**/
void combination_state_synchronization(u8* lamp_buff , u8 * combination_state)
{
	u8 identical_flag, contrary_flag;
	/* ���������һ�£�״̬Ϊ1�����������ȡ��һ�£�״̬Ϊ0 ������������ı�״̬ */	
	for(u8 i = 0 ; i < 16 ; i++)
	{
		/*��״̬λ*/
		identical_flag =  0;
		contrary_flag  =  0;
		/*�ж�40����״̬ ����ֻ������38������*/
		for(u8 j = 0 ; j < 38 ; j++)
		{
			if( user_system_data.scene.scene_data[16 + i].data[11 + j] != 0xff) /*������nc״̬*/
			{
				/*�Ƚϳ���ֵ�Ƿ�͵�״̬��ͬ*/
				if( (!user_system_data.scene.scene_data[16 + i].data[11 + j]) != (!lamp_buff[j]) )
					identical_flag = 1;
				else
					contrary_flag = 1;			
			}
		}		
	}		
}
/**
*********************************************************************************************************
* @����	: 
* @����	:���淢������ 
*********************************************************************************************************
**/
void rs485inside_release_data_to_cache(u8* buff, u8 num ,data_container_t  *current_data_container , u32 delay)
{
	directional_release_data_to_cache( &logic_business_release_buff ,
		current_data_container, 
		buff , 
		num , 
		(u8*)"logic_rs485inside", 
		sizeof("logic_rs485inside"),
		delay);  /*�ڲ��Ƶ�Э�鷢�͹̶���ʱ*/
	/*��������*/	
	if( 1 == ethernet_debug_flag )		
		debug_ethernet_transmission(buff , num, 0x71, 0x7b);			
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��������ת�� 
*********************************************************************************************************
**/
const u8 service_maping[8] ={0,4,3,6,2,7,1,5};
void service_transformation(u8* key_buff)
{
	u8 *service_pointer = &current_user.service.card_power_on_status;
	if(key_buff[STATUS_BYTE] == 1) /*��1*/
	{
		service_pointer[ service_maping[ key_buff[DATA_BYTE] - 20 ] ] = 1;		
	}
	else if(key_buff[STATUS_BYTE]  == 2)/*��0*/
	{
		service_pointer[ service_maping[ key_buff[DATA_BYTE] - 20 ] ] = 0;
	}		
}
/**
*********************************************************************************************************
* @����	: 
* @����	:����ָ�� ��� 
*********************************************************************************************************
**/
void service_f0_command_package(u8* package_buff, service_status_t* service_buff )
{
/*�ؼ����ֲ��㹫����ɾ������*/
}
/**
*********************************************************************************************************
* @����	: 
* @����	:����
*********************************************************************************************************
**/
static void hotle_agreement_music(u8* key_buff)
{	
/*�ؼ����ֲ��㹫����ɾ������*/
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�Ƶ�Э��ƴ���    
*********************************************************************************************************
**/
static void hotle_agreement_lamp_handle(u8* key_buff, data_container_t  *current_data_container)
{
	u8 instruction_packaging_buff[60];
	u16 lamp_delay;	
	/*�Ƹ�����״̬*/
	if(key_buff[STATUS_BYTE] == 0)/*��ȡ��*/
		current_user.lamp.lamp_buff[ key_buff[DATA_BYTE] - KEY_VALUE_LAMP_START] = !	current_user.lamp.lamp_buff[ key_buff[DATA_BYTE] - KEY_VALUE_LAMP_START];
	/*�ƿ�Ϊ100 ���ݵ����*/
	if(current_user.lamp.lamp_buff[ key_buff[DATA_BYTE] - KEY_VALUE_LAMP_START] == 1)
		current_user.lamp.lamp_buff[ key_buff[DATA_BYTE] - KEY_VALUE_LAMP_START] = 100;
	/*��ָ����  ͳһʹ�ó���һ����ʱ*/ 
	lamp_delay = user_system_data.scene.scene_data[0].data[9]*256 + user_system_data.scene.scene_data[0].data[10];
	lamp_f1_command_package(instruction_packaging_buff, &current_user.lamp , lamp_delay , FORMAT_MS , user_system_data.set.agreement_version );	
	/*������������  ֡ͷ ���� ��ʱ ��״̬ */
	rs485inside_release_data_to_cache(instruction_packaging_buff, (user_system_data.set.agreement_version == 0) ? 51 : 50 , current_data_container, PROGRAM_FIXED_DELAY);//��ʱ�������� ������ʱ
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�Ƶ�Э�鳡������  
*********************************************************************************************************
**/
static void hotle_agreement_scene_handle(u8* key_buff, data_container_t  *current_data_container)
{
	u8 instruction_packaging_buff[60];
	u16 lamp_delay;	
	/*����������״̬  �Իָ��;��Կ�ִ��*/
	if( (key_buff[STATUS_BYTE] == 0) || (key_buff[STATUS_BYTE] == 1) )
	{
		current_user.lamp.mode = key_buff[DATA_BYTE] - KEY_VALUE_SECEN_START;
		/*���µ�״̬*/
		lamp_delay = scene_mode_lamp_update_status(&current_user.lamp, &user_system_data.scene.scene_data[ current_user.lamp.mode ] );
		/*��ָ����*/
		lamp_f1_command_package(instruction_packaging_buff, &current_user.lamp , lamp_delay , FORMAT_MS, user_system_data.set.agreement_version );	
		/*������������  ֡ͷ ���� ��ʱ ��״̬*/	
		rs485inside_release_data_to_cache(instruction_packaging_buff, (user_system_data.set.agreement_version == 0) ? 51 : 50, current_data_container , PROGRAM_FIXED_DELAY); //��ʱ�������� ������ʱ
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�Ƶ�Э����ϴ���   
*********************************************************************************************************
**/
static u8 combination_state_buff[16];
static void hotle_agreement_combination_handle(u8* key_buff, data_container_t  *current_data_container)
{

	u8 instruction_packaging_buff[60];
	u16 lamp_delay;	
	/*��ϸ�����״̬  �Իָ�ȡ��  ��  �ߵ���Ч���Կ���ִ��*/
	if( (key_buff[STATUS_BYTE] == 0) || (key_buff[STATUS_BYTE] == 1) || (key_buff[STATUS_BYTE] == 2) )
	{
		u8 combination_num;
		combination_num = key_buff[DATA_BYTE]-KEY_VALUE_COMBINATION_START;
		combined_byte_mapping(&current_user.lamp, combination_num ,key_buff[STATUS_BYTE]);
		if(key_buff[STATUS_BYTE] == 0)
			combination_state_buff[combination_num] = !combination_state_buff[combination_num];			
		/*���µ�״̬*/
		lamp_delay =combination_lamp_update_status(&current_user.lamp, &user_system_data.scene.scene_data[ key_buff[DATA_BYTE] - KEY_VALUE_SECEN_START] ,combination_state_buff[combination_num]);
		/*��ָ����*/
		lamp_f1_command_package(instruction_packaging_buff, &current_user.lamp, lamp_delay , FORMAT_MS, user_system_data.set.agreement_version );		
		/*������������  ֡ͷ ���� ��ʱ ��״̬ */
		rs485inside_release_data_to_cache(instruction_packaging_buff, (user_system_data.set.agreement_version == 0) ? 51 : 50, current_data_container , PROGRAM_FIXED_DELAY); //��ʱ��������
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�Ƶ�Э�������     
*********************************************************************************************************
**/
static void hotle_agreement_service_handle(u8* key_buff, data_container_t  *current_data_container)
{
	u8 instruction_packaging_buff[60];	
	/*��������ת����������Ϣ*/
	service_transformation(key_buff);
	/*���������ָ��*/
	service_f0_command_package(instruction_packaging_buff,&current_user.service);
	/*��������״̬ 9�ֽ�*/
	rs485inside_release_data_to_cache(instruction_packaging_buff, 9, current_data_container , 50);	//��ʱ�������� 500ms��ʱ ���ӷ���ָ�����ʱ������忨ָ��ͷ���ָ���ʱ���
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�Ƶ�Э�鴦���ƣ���������ϣ�����   key_buff[STATUS_BIT]Ϊ״̬    key_buff[DATA_BYTE]Ϊ��ֵ 
*********************************************************************************************************
**/
static void hotle_agreement_handle(u8* key_buff, data_container_t  *current_data_container)
{
	if((key_buff[DATA_BYTE] >= KEY_VALUE_LAMP_START) && (key_buff[DATA_BYTE] <= KEY_VALUE_LAMP_END))/*�Ʋ���*/
	{	
		hotle_agreement_lamp_handle(key_buff, current_data_container);
	}
	else if((key_buff[DATA_BYTE] >= KEY_VALUE_SECEN_START) && (key_buff[DATA_BYTE] <= KEY_VALUE_SECEN_END))/*���� ģʽ 192~207  */
	{
		hotle_agreement_scene_handle(key_buff, current_data_container);
	}
	else if(key_buff[DATA_BYTE] == 30)/* ����*/
	{
		if(key_buff[STATUS_BYTE] == 1 ) 
			current_user.service.door_lock_status = 1;
		else if(key_buff[STATUS_BYTE] == 2 )
			current_user.service.door_lock_status = 0;
	}
	else if(( key_buff[DATA_BYTE] >= 170 ) && (key_buff[DATA_BYTE] <= 177))/* ����ָ��*/
	{
		hotle_agreement_music( key_buff );
	}	

	/* ͬ�����״̬ */
	combination_state_synchronization( current_user.lamp.lamp_buff , combination_state_buff);	
}

/**
*********************************************************************************************************
* @����	: 
* @����	:�ڲ�485�����
*********************************************************************************************************
**/
void inside_lookup_handle(u8* key_buff,u8 type , data_container_t  *current_data_container)
{
	if(type != 1)
	{
		/*���β��*/
		for(u8 i=0; i < MACTH_TABLE_NUM; i++)
		{
			/*ƥ���ֵ ����Ϊ���Թ�*/			
			if( (key_buff[DATA_BYTE] == user_system_data.logic.inside_table[i].value ) && (key_buff[STATUS_BYTE] != 2 ) )
			{

				if(user_system_data.logic.inside_table[i].length != 0)
				{
					/*��������ѡ�����ͨ��*/
					u8 name_buff[20];
					u8 name_length;
					if(user_system_data.logic.inside_table[i].type == 0)
					{					
						name_length = sizeof("logic_rs485inside");
						memcpy(name_buff , "logic_rs485inside" ,sizeof("logic_rs485inside"));
						/*������Ϣ�ϱ�*/	
						if( 1 == ethernet_debug_flag )	
							debug_ethernet_transmission(user_system_data.logic.inside_table[i].buff , user_system_data.logic.inside_table[i].length , 0x71, 0x7b);
						
					}				
					else
					{
						name_length = sizeof("logic_rs485all");
						memcpy(name_buff , "logic_rs485all" ,sizeof("logic_rs485all"));
						/*������Ϣ�ϱ�*/
						if( 1 == ethernet_debug_flag )	
							debug_ethernet_transmission(user_system_data.logic.inside_table[i].buff , user_system_data.logic.inside_table[i].length , 0x73, 0x7b);							
					}
					/*���򷢲���Ϣ*/
					directional_release_data_to_cache( 
					&logic_business_release_buff ,
					current_data_container, 
					user_system_data.logic.inside_table[i].buff , 
					user_system_data.logic.inside_table[i].length , 
					name_buff, 
					name_length,
					(user_system_data.logic.inside_table[i].delay[0]*256 + user_system_data.logic.inside_table[i].delay[1])/10);	//��ʱ��������   ms
					
				}				
					
			}
		}
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�ⲿ485�����
*********************************************************************************************************
**/
void outside_lookup_handle(u8* key_buff,u8 type, data_container_t  *current_data_container)
{
	if(type != 2)
	{
		/*���β��*/		
		for(u8 i=0; i < MACTH_TABLE_NUM; i++)
		{	
			/*ƥ���ֵ*/			
			if( (key_buff[DATA_BYTE] == user_system_data.logic.outside_table[i].value) && (key_buff[STATUS_BYTE] != 2 ) )
			{
				if(user_system_data.logic.outside_table[i].length != 0)
				{
					/*��������ѡ�����ͨ��*/
					u8 name_buff[20];
					u8 name_length;
					if(user_system_data.logic.outside_table[i].type == 0)
					{					

						name_length = sizeof("logic_rs485inside");
						memcpy(name_buff , "logic_rs485inside" ,sizeof("logic_rs485inside"));
						
						/*������Ϣ�ϱ�*/
						if( 1 == ethernet_debug_flag )
							debug_ethernet_transmission(user_system_data.logic.outside_table[i].buff , user_system_data.logic.outside_table[i].length , 0x71, 0x7b);	
					}				
					else
					{
						name_length = sizeof("logic_rs485all");
						memcpy(name_buff , "logic_rs485all" ,sizeof("logic_rs485all"));
						/*������Ϣ�ϱ�*/
						if( 1 == ethernet_debug_flag )
							debug_ethernet_transmission(user_system_data.logic.outside_table[i].buff , user_system_data.logic.outside_table[i].length , 0x73, 0x7b);							
					}
					/*���򷢲���Ϣ*/
					directional_release_data_to_cache( 
					&logic_business_release_buff ,
					current_data_container, 
					user_system_data.logic.outside_table[i].buff , 
					user_system_data.logic.outside_table[i].length , 
					name_buff, 
					name_length,
					(user_system_data.logic.outside_table[i].delay[0]*256 + user_system_data.logic.outside_table[i].delay[1])/10);	//��ʱ��������	 ms		
				}
			}
		}
	}
}

/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
void hotel_air_conditioner_package_send(air_conditioner_t *air_conditioner )
{
/*�ؼ����ֲ��㹫����ɾ������*/
}
/**
*********************************************************************************************************
* @����	: 
* @����	:
*********************************************************************************************************
**/
void analysis_of_air_conditioning_instructions(data_container_t* data_container)
{
	if( ( strstr("rs485inside"  , (char*)data_container->name ) ) != NULL )  /*�жϷ������Ƿ�Ϊrs485inside*/
	{
		/*�жϿյ�ָ��*/
		if((data_container->buff[0] == 0xf4) && (data_container->buff[1] == 0xf3) )
		{
			/* 2���յ� */
			if(data_container->buff[2] < 2)
			{
				u8 add = data_container->buff[2];
				
				current_user.air_conditioner[add].set_temperature = data_container->buff[3] ;
				current_user.air_conditioner[add].wind_speed = data_container->buff[4] ;
				current_user.air_conditioner[add].wind_direction = data_container->buff[5] ;
				current_user.air_conditioner[add].power = data_container->buff[6] ;
				current_user.air_conditioner[add].mode = data_container->buff[8] ;
				current_user.air_conditioner[add].real_temperature = data_container->buff[9] ;
				
				key_value_updata_flag	= 1;			
			}
		}
	}
}
/***********************************************END*****************************************************/

