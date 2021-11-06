/**
*********************************************************************************************************
*                                        		gateway
*                                      (c) Copyright 2021-2031
*                                         All Rights Reserved
*
* @File    : 
* @By      : liwei
* @Version : V0.01
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
#include "rs458_task.h"
#include "rs458_bsp.h"
#include "rs458_analysis.h"
#include "string.h"
#include "stdlib.h"
#include "debug_bsp.h"
#include "broker_task.h"
#include "user_type.h"
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

/*
*********************************************************************************************************
Variables
*********************************************************************************************************
*/
TaskHandle_t rs485_task_handler;
static uint8_t receive_asc_buff[CACHE_BUFF_NUM*2];

business_information_t  rs485_business=
{
	.release_name = "rs485inside",
	.subscribe_name = "logic_rs485inside,logic_rs485outside,logic_rs485all",		
	.release_data.serial =0,
	.large_data_length = 0,
};
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
void rs485_inside_analysis_handle(void);
void rs485_outside_analysis_handle(void);
void rs485_send_handle(void);
/**
*********************************************************************************************************
* @����	: 
* @����	: 
*********************************************************************************************************
**/
void rs485_task(void *pvParameters)
{
	static uint16_t clk=0;
	rs485_usart_init();
	for(;;)
	{	
		if(((clk++) % 1000) == 0)	
			debug_printf("�û�����RS485����������...\n");/*��ӡ��Ϣ*/	
		/*������3 ��������*/		
		data_analysis_handle(&usart3_commnuication_frame,rs485_inside_analysis_handle);
		/*������4 ��������*/			
		data_analysis_handle(&usart4_commnuication_frame,rs485_outside_analysis_handle);
		/*����3 ����4 ��������*/			
		rs485_send_handle();
		vTaskDelay(5);   
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	: 
*********************************************************************************************************
**/
void rs485_inside_analysis_handle(void)
{
	static u8 release_num=1;
	release_num++;
	if(release_num == 0)
		release_num = 1;
	/*ת���ַ���*/			
	hex_to_ascii(usart3_commnuication_frame.analysis_buff,receive_asc_buff,usart3_commnuication_frame.analysis_long);
	/*��ӡ���*/
	debug_printf("485 inside�յ����ݣ�%s\n",receive_asc_buff);
	memset(receive_asc_buff,0,CACHE_BUFF_NUM*2);
	/*������Ϣ ����*/
	directional_release_data(&rs485_business, usart3_commnuication_frame.analysis_buff, usart3_commnuication_frame.analysis_long ,(u8*)"rs485inside", sizeof("rs485inside"));	
}
/**
*********************************************************************************************************
* @����	: 
* @����	: 
*********************************************************************************************************
**/
void rs485_outside_analysis_handle(void)
{
	static u8 release_num=1;
	release_num++;
	if(release_num == 0)
		release_num = 1;
	/*ת���ַ���*/			
	hex_to_ascii(usart4_commnuication_frame.analysis_buff,receive_asc_buff,usart4_commnuication_frame.analysis_long);
	/*��ӡ���*/
	debug_printf("485 outside�յ����ݣ�%s\n",receive_asc_buff);
	memset(receive_asc_buff,0,CACHE_BUFF_NUM*2);
	
	/*������Ϣ ����*/
	directional_release_data(&rs485_business, usart4_commnuication_frame.analysis_buff, usart4_commnuication_frame.analysis_long ,(u8*)"rs485outside", sizeof("rs485outside"));
}

/**
*********************************************************************************************************
* @����	: 
* @����	: 
*********************************************************************************************************
**/
void rs485_send_handle(void)
{
	u8 subscribe = SUBSCRIBE_NULL;
	/*�ж϶�����Ϣ�Ƿ�������*/
	subscribe = check_subscribe_information(&rs485_business);
	if( subscribe != SUBSCRIBE_NULL )
	{
		/*���ݲ�ͬ��ͨ������������*/
		if( ( ( strstr("logic_rs485inside"  ,(char*)rs485_business.subscribe_data_buff[subscribe].name ) ) != NULL ) ||
		(( strstr("logic_rs485all"  ,(char*)rs485_business.subscribe_data_buff[subscribe].name ) ) != NULL )) 
		{
			rs485_inside_send_buff(rs485_business.subscribe_data_buff[subscribe].buff, rs485_business.subscribe_data_buff[subscribe].length);
		}
		if( ( ( strstr("logic_rs485outside"  ,(char*)rs485_business.subscribe_data_buff[subscribe].name ) ) != NULL ) ||
		(( strstr("logic_rs485all"  ,(char*)rs485_business.subscribe_data_buff[subscribe].name ) ) != NULL )) 			
		{
			rs485_outside_send_buff(rs485_business.subscribe_data_buff[subscribe].buff, rs485_business.subscribe_data_buff[subscribe].length);
		}
		/*�����Ϣ*/
		clear_release_information(&rs485_business.subscribe_data_buff[subscribe]);
	}
}
/***********************************************END*****************************************************/

