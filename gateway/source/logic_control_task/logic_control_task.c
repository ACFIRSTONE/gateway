
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
/*ҵ���ķ����ṹ����*/
business_information_t  logic_business=
{
	.release_name = "logic",
	.subscribe_name = "rs485inside,rs485outside,gpio,ethernet,local_ethernet",		
	.release_data.serial =0,
	.large_data_length = 0,
};

key_value_frame_t user_key_data;
key_value_frame_t user_key_analysis;
key_value_frame_t user_key_execute;

key_value_delay_frame_t  user_delay_key;
static data_container_t  current_data_container;

system_information_t user_system_data;

release_buff_t logic_business_release_buff=
{
	.pointer = 0,
};
u8 *user_system_data_point =  (u8*)&user_system_data;
user_total_information_t  current_user;
u8 key_value_updata_flag =0;
extern u16 ethernet_data_monitor_num ;
user_logic_t user_logic;
u8 user_uid[3];
extern u8 firmware_version ;
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
void communication_485_analysis(data_container_t* data_container);
static void dry_contac_input_analysis(data_container_t* data_container);
void ethernet_send_hearbeat(data_container_t* current_data_container);
void id_transformation(u8 *buff, u8 *id_buff);
void ethernet_data_analysis(data_container_t* data_container ,data_container_t* current_data_container);
void key_value_analysis_form_subscribe(data_container_t* data_container);
void communication_485_handle(u8* key_buff ,u8 type, data_container_t  *current_data_container);
static void delay_data_release(void);
void wdt_config(void);
void read_uid_data(void);
void logic_control_time_int(interrupt_callback user_handle);
void logic_control_interrupt_callback(void );
void analysis_of_air_conditioning_instructions(data_container_t* data_container);
void key_value_logic_handle(void );
void key_card_power_off(void);

/**
*********************************************************************************************************
* @����	: 
* @����	:���������û�������߼� �������ݴ�����Ӳ������
*********************************************************************************************************
**/
void logic_control_task(void *pvParameters)
{
	static uint16_t clk=0;
	/*��FLAHS��ȡϵͳ����*/
	user_read_system_data(); 
	/*��ȡUID*/
	read_uid_data();
	/*�û�����������*/	
	key_buff_clear();
	memset(&current_user, 0,sizeof(current_user));
	/*��ʼ����ʱ ��ע���жϻص�����*/
	logic_control_time_int( logic_control_interrupt_callback );
	/*���Ź�����*/
	wdt_config();
	/*�ص������豸*/
	key_card_power_off();	
	while(1)
	{		
		/*��ֵ�����������ֵ*/
		key_value_analysis_cache();
		/*��ֵ�߼�����*/
		key_value_logic_handle();		
		/*ִ�м�ֵ*/		
		key_value_execute();	
		/*��̫��������������*/
		ethernet_send_hearbeat(&current_data_container);
		/*ι��*/		
		WDT_RefreshCounter();
		/*��ӡ��Ϣ*/	
		if((clk++%3000) == 0) 
			debug_printf("�߼���������������...\n");	
		/*��ʱ*/
		vTaskDelay(3);		
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�߼������жϻص����� 100ms�ж� �����뼶��ʱ 
*********************************************************************************************************
**/
void logic_control_interrupt_callback(void)
{
	/*��ʱ����ɨ���ѯ*/
	key_delay_scanf_handle();
	/*�ӻ����з������ݲ���ʱ*/
	delay_data_release();	
}
/**
*********************************************************************************************************
* @����	: 
* @����	: ��ȡUID   
*********************************************************************************************************
**/
void read_uid_data(void)
{
	u32 num;
	num = M4_EFM->UQID3;
	user_uid[2] = num>>16;
	user_uid[1] = num>>8;
	user_uid[0] = num;		
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�ӻ����з������ݲ���ʱ
*********************************************************************************************************
**/
static void delay_data_release(void)
{
	for(u8 i = 0 ; i < RELEASE_BUFF_MAX ; i++)
	{
		/*�жϻ����Ƿ�������*/
		if(logic_business_release_buff.release[i].serial != 0)
		{
			/*�ж���ʱ*/
			if(logic_business_release_buff.delay[i] > 0 )
			{
				logic_business_release_buff.delay[i]--;
			}
			else
			{
				/*�жϷ��������Ƿ�Ϊ�� ֻ��Ϊ��ʱ�Ų�������Ḳд*/
				if(logic_business.release_data.serial == 0) 
				{
					memcpy(logic_business.release_name, &logic_business_release_buff.release[i].name, sizeof(logic_business.release_data.name)); /*���ķ�������*/
					/*��������*/
					memcpy(logic_business.release_data.name, &logic_business_release_buff.release[i].name, sizeof(logic_business.release_data.name));
					memcpy(logic_business.release_data.buff, &logic_business_release_buff.release[i].buff, sizeof(logic_business.release_data.buff));
					logic_business.release_data.length = logic_business_release_buff.release[i].length;
					logic_business.release_data.serial = logic_business_release_buff.release[i].serial;
					/*�������� */
					memset(&logic_business_release_buff.release[i], 0, sizeof(data_container_t));
					/*��ɷ��;����� �������жϷ��� */
					break;
				}							
			}	
		}
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ֵ����
*********************************************************************************************************
**/
static void key_value_analysis_cache(void)
{
	/*��ȡ�������ݲ�ִ�лص�����*/
	read_subscribe_payload(&logic_business , key_value_analysis_form_subscribe);	
}
/**
*********************************************************************************************************
* @����	: 
* @����	:����������Ϣ�еļ�ֵ 
*********************************************************************************************************
**/
void key_value_analysis_form_subscribe(data_container_t* data_container)
{
	/*485 ��ֵ����*/
	communication_485_analysis(data_container);
	/*�忨�ж� */
	dry_contac_input_analysis(data_container);/*�ɽӵ��ֵ����*/
	/*��̫�����ݽ���*/
	ethernet_data_analysis(data_container, &current_data_container);	
	/*�����յ�ָ��*/
	analysis_of_air_conditioning_instructions(data_container);			
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�ɽӵ�����
*********************************************************************************************************
**/
static void dry_contac_input_analysis(data_container_t* data_container)
{
	u8 key_buff[3];
	if( ( strstr("gpio"  ,(char*)data_container->name ) ) != NULL )  /*�жϷ������Ƿ�Ϊbutton*/
	{
		/*buff[0]ͨ��   buff[1] �ߵ͵�ƽ*/
		u8 i = data_container->buff[0];  /*����ͨ��   ���������ж�*/	
		/*һ���������ͱ仯 ��Ȼ���� �Իָ�*/
		//if((user_system_data.logic.input[i].type == 1) || (user_system_data.logic.input[i].type == 3))
		/*�Իָ��� һ���������ͱ仯 ��Ȼ����*/
		if( user_system_data.logic.input[i].type == 1 )
		{
			if( data_container->buff[1] == 0)
			{
				/*����3�ֽڼ�ֵ*/
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 0;/*ȡ��*/
				key_buff[DATA_BYTE] =user_system_data.logic.input[i].value;				
				key_buff_write(key_buff, 0);
			}				
		}
		/*�Իָ��� һ���������ͱ仯 ��Ȼ����*/
		else if(user_system_data.logic.input[i].type == 3 )
		{
			if( data_container->buff[1] == 1)
			{
				/*����3�ֽڼ�ֵ*/
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 0;/*ȡ��*/
				key_buff[DATA_BYTE] =user_system_data.logic.input[i].value;				
				key_buff_write(key_buff, 0);
			}		
		}			
		/*����Ч ���Կ���*/
		else if(user_system_data.logic.input[i].type == 0 )
		{
			if( data_container->buff[1] == 1)
			{		
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 1;/*1 ��*/
				key_buff[DATA_BYTE] =user_system_data.logic.input[i].value;
				key_buff_write(key_buff, 0);
			}
			else
			{			
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 2;/*2 ��*/
				key_buff[DATA_BYTE] =user_system_data.logic.input[i].value;
				key_buff_write(key_buff, 0);					
			}
		}
		/*����Ч ���Կ���*/
		else if(user_system_data.logic.input[i].type == 2 )
		{
			if( data_container->buff[1] == 0)
			{
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 1;/*1 ��*/
				key_buff[DATA_BYTE] =user_system_data.logic.input[i].value;
				key_buff_write(key_buff, 0);
			}
			else
			{
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 2;/*2 ��*/
				key_buff[DATA_BYTE] =user_system_data.logic.input[i].value;
				key_buff_write(key_buff, 0);					
			}
		}			
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ֵ����
*********************************************************************************************************
**/
static void key_value_execute(void)
{
	u8 key_buff[3],type;
	/*��ֵ���治Ϊ��*/
	if(key_buff_check())
	{
		/*��ֵ���������ϱ��ƶ�*/
		key_value_updata_flag = 1;
		/*��ȡ��ֵ������*/
		type = key_buff_read(key_buff);
		/*ͨѶ��ֵ����*/
		communication_485_handle(key_buff, type , &current_data_container);
		/*�����ֵ����*/
		output_control_handle(key_buff);	
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�������   ʹ�ü�ֵBUFF[0] ��һ����ʱ��ֵ ͬʱ  BUFF[2] ΪFF
*********************************************************************************************************
**/
static void output_control_handle(u8* key_buff)
{
	for(u8 i=0; i < 10; i++)
	{
		if(compare_hex_buff(&key_buff[DATA_BYTE], &user_system_data.logic.output[i] ,1))/*  ���Ƚ� 0λ���λ  */
		{
			
			memcpy(current_data_container.name, "logic_output", sizeof("logic_output"));
			
			current_data_container.buff[0] =i;
			current_data_container.buff[1] =key_buff[STATUS_BYTE];
			
			current_data_container.length = 2;
			current_data_container.serial = 1;	
			release_buff_write( &logic_business_release_buff, &current_data_container, 5);	
			break;
		}
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�û����ݳ�ʼ��
*********************************************************************************************************
**/
void uesr_data_reset(void)
{
	memcpy(user_system_data.set.domain_name,  "admin.hotelyun.net", sizeof("admin.hotelyun.net"));
	user_system_data.set.domain_name_length = sizeof("admin.hotelyun.net");
	user_system_data.set.domain_name_port[0] = 5188/256;
	user_system_data.set.domain_name_port[1] = 5188%256;
	
	user_system_data.set.load_balancing_ip[0] = 47;
	user_system_data.set.load_balancing_ip[1] = 106;
	user_system_data.set.load_balancing_ip[2] = 234;
	user_system_data.set.load_balancing_ip[3] = 88;
	
	user_system_data.set.load_balancing_port[0] = 5188/256;
	user_system_data.set.load_balancing_port[1] = 5188%256;	
	
	for(u8 i = 0 ; i<10 ; i++)
	user_system_data.check[i] = i;
}
/***********************************************END*****************************************************/

