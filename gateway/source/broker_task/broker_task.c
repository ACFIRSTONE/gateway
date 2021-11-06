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
#include "broker_task.h"
#include "user_type.h"
/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
#define BROKER_AMOUNT   4

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
extern business_information_t  rs485_business;
extern business_information_t  logic_business;
extern business_information_t  gpio_business;
extern business_information_t  ethernet_business;
/*������Ϣһ��Ҫ��д��ȷ������һ��Ҫ��ȷ����Ȼ�����ܷ�*/
business_information_t* broker_information[BROKER_AMOUNT] =
{
	&rs485_business,
	&logic_business,	
	&gpio_business,
	&ethernet_business,
};
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
void clear_release_information(data_container_t* release);
void broker_push_information(u8 release);
void broker_handle(void);
void clear_broker_information(void);

/**
*********************************************************************************************************
* @����	: 
* @����	:���������񣬸��ƶ�ȡÿ��ҵ��ķ������ݣ������������͸���ض�����  
*********************************************************************************************************
**/
void broker_task(void *pvParameters)
{
	clear_broker_information();/*��մ���ҵ�����Ϣ */
	while(1)
	{
		broker_handle();/*��������Ϣ���ʹ���*/
		vTaskDelay(1); 
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��մ���ҵ�����Ϣ 
*********************************************************************************************************
**/
static void clear_broker_information(void)
{
	for( u8 i = 0; i < BROKER_AMOUNT; i++ ) /*�����������ҵ������*/
	{
		/*��շ�����Ϣ�����к�*/
		memset( broker_information[i]->release_data.buff, 0 , sizeof( broker_information[0]->release_data.buff ) );
		broker_information[i]->release_data.serial = 0;	
		/*��ն�����Ϣ�����к�*/
		for( u8 k = 0; k < (SUBSCRIBE_AMOUNT - 1); k++ )
		{
			memset( broker_information[i]->subscribe_data_buff[ k ].buff, 0 , sizeof( broker_information[0]->subscribe_data_buff[ 0 ].buff ) );
			broker_information[i]->subscribe_data_buff[ k ].serial = 0;	
		}			
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:���������鴦��������Ϣ
*********************************************************************************************************
**/
static void broker_handle(void)
{
	for( u8 i = 0; i < BROKER_AMOUNT; i++ ) /*������ѯ����ҵ��*/
	{
		if( broker_information[i]->release_data.serial != 0 ) /*�жϸ�ҵ�񷢲������Ƿ�Ϊ0*/	
		{
			/*������Ϣ��������*/
			broker_push_information( i );
			/*��շ�������*/
			clear_release_information(&broker_information[i]->release_data);
		}	
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:������Ϣ�������� 
*********************************************************************************************************
**/
static void broker_push_information(u8 release)
{
	for( u8 i = 0; i < BROKER_AMOUNT; i++)  /*������ѯ����ҵ��*/	
	{
		if( i != release) /*����Ϣ���͸�����ҵ��*/
		{
			if( ( strstr((char*)broker_information[i]->subscribe_name  ,(char*)broker_information[release]->release_name ) ) != NULL )  /*�жϷ������Ƿ��ڶ�������*/
			{
				for( u8 k = 0; k < (SUBSCRIBE_AMOUNT - 1); k++ )/*������ѯ���ж��Ļ����Ƿ��п�λ*/
				{
					if( broker_information[i]->subscribe_data_buff[k].serial == 0 ) /*�ж϶��Ļ����к��Ƿ�Ϊ0 ��0Ϊ��*/
					{
						/*�������� */
						memcpy( broker_information[i]->subscribe_data_buff[k].buff, broker_information[release]->release_data.buff ,CONTAINER_AMOUNT);						
						/*����*/
						memcpy( broker_information[i]->subscribe_data_buff[k].name, broker_information[release]->release_name , sizeof(broker_information[0]->subscribe_data_buff[0].name));						
						/*���к�*/
						broker_information[i]->subscribe_data_buff[k].serial = broker_information[release]->release_data.serial;
						/*���ݳ���*/
						broker_information[i]->subscribe_data_buff[k].length = broker_information[release]->release_data.length;	
						/*���Ƴ�����������*/
						if((broker_information[i]->large_data_length != 0 )&& ( broker_information[i]->large_data_length >=broker_information[release]->large_data_user_length))	
							memcpy(broker_information[i]->large_data_buff ,broker_information[release]->large_data_buff,broker_information[release]->large_data_user_length);	
						/*�������������㣬����*/
						broker_information[release]->large_data_user_length = 0;
						break;
					}
				}		
			}										
		}
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�����Ϣ 
*********************************************************************************************************
**/
void clear_release_information(data_container_t* release)
{
	/*��շ�����Ϣ�����к�*/
	memset( release->buff, 0, sizeof( release->buff ));
	memset( release->name, 0, sizeof( release->name ));	
	release->serial = 0;
	release->length = 0;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ⶩ����Ϣ 
*********************************************************************************************************
**/
u8 check_subscribe_information(business_information_t* business)
{
	for(u8 i=0 ; i<(SUBSCRIBE_AMOUNT - 1) ;i++)
	{
		if(business->subscribe_data_buff[i].serial !=0) /*��ⶩ�Ļ�������кŲ�Ϊ��*/
			return i;			
	}
	return SUBSCRIBE_NULL;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�������� 
*********************************************************************************************************
**/
void release_buff_write(release_buff_t* release_buff, data_container_t* data , u32 delay)
{
	for(u8 i = 0 ; i < RELEASE_BUFF_MAX ; i++)
	{
		/*�жϻ����Ƿ��*/
		if(release_buff->release[i].serial == 0) /*����ָ��С���������*/
		{
			memcpy(&release_buff->release[i], data, sizeof(data_container_t)); /*�����������ݿ鵽����*/
			release_buff->delay[i] =  delay; /*��ʱ*/
			break;
		}
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�������ݶ�ȡ  ���ӷ���ֵ ����ֵΪ��ʱ
*********************************************************************************************************
**/
u16 release_buff_check(release_buff_t* release_buff, business_information_t* data)
{
	u16 return_data=0;
	if(release_buff->pointer > 0) /*�жϷ��������Ƿ�Ϊ��*/
	{			
		if(data->release_data.serial == 0) /*�жϷ��������Ƿ�Ϊ��*/
		{
			memcpy(data->release_name, &release_buff->release[release_buff->pointer].name, sizeof(data->release_data.name)); /*���ķ�������*/
			/*��������*/
			memcpy(data->release_data.name, &release_buff->release[release_buff->pointer].name, sizeof(data->release_data.name));
			memcpy(data->release_data.buff, &release_buff->release[release_buff->pointer].buff, sizeof(release_buff->release[0].buff));
			data->release_data.length = release_buff->release[release_buff->pointer].length;
			data->release_data.serial = release_buff->release[release_buff->pointer].serial;
			return_data	=	 release_buff->delay[release_buff->pointer];
			/*�������� �� ָ���Լ�*/
			memset(&release_buff->release[release_buff->pointer], 0, sizeof(data_container_t));
			release_buff->pointer--; 		
		}
	}
	return return_data;
}
/**
*********************************************************************************************************
* @����	: 
* @����	: ��ȡ������Ϣ ��ִ����ػص�����
*********************************************************************************************************
**/
void read_subscribe_payload(business_information_t* business, business_func_t  business_func)
{
	u8 subscribe = SUBSCRIBE_NULL;
	subscribe = check_subscribe_information(business);/*��ⶩ����Ϣ�Ƿ�Ϊ��*/
	/*�ж϶����ڲ���Ϊ�� ִ�лص�����*/
	if( subscribe != SUBSCRIBE_NULL )
	{	
		(*business_func)(&business->subscribe_data_buff[subscribe]);/*ִ�лص�����*/
		clear_release_information(&business->subscribe_data_buff[subscribe]);/*��ջ���*/
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:���򷢲����ݵ�����
*********************************************************************************************************
**/
void directional_release_data_to_cache(release_buff_t* business_release_buff, data_container_t* current_data_container,u8* release_data,u8 release_length, u8* release_name, u8 release_name_length ,u32 dalay)
{
	memcpy(current_data_container->name, release_name, release_name_length);	
	memcpy(current_data_container->buff, release_data, sizeof(current_data_container->buff));
	current_data_container->length = release_length;
	current_data_container->serial = 1;	
	release_buff_write( business_release_buff, current_data_container, dalay);	/*��ʱ*/
}
/**
*********************************************************************************************************
* @����	: 
* @����	:���򷢲�����
*********************************************************************************************************
**/
void directional_release_data( business_information_t* current_data_container, u8* release_data,u8 release_length, u8* release_name, u8 release_name_length )
{
	/*������Ϣ ����*/
	memcpy(current_data_container->release_name, release_name, release_name_length);
	memcpy(current_data_container->release_data.buff, release_data, release_length); 
	/*������Ϣ ���*/
	current_data_container->release_data.serial = 1;	
	/*������Ϣ ���ݳ���*/
	current_data_container->release_data.length = release_length;	
}
/***********************************************END*****************************************************/

