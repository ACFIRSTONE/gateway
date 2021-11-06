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
#include "rs458_bsp.h"
#include "rs458_analysis.h"

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
usart_frame_t   usart3_commnuication_frame;   /*insied*/
usart_frame_t   usart4_commnuication_frame;   /*outsied*/
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/


/**
*********************************************************************************************************
* @����	: 
* @����	: ˢ�´�����ʱ,��Ž�������
*********************************************************************************************************
**/
void common_receive_refresh(uint8_t receive_data, usart_frame_t *com_pointer)
{
	//ˢ�¼�����
	com_pointer->refresh_num=0;
	//��������
	if(com_pointer->current_num<(CACHE_BUFF_NUM-1))
		com_pointer->receive_buff[com_pointer->current_num++]=receive_data;		
}
/**
*********************************************************************************************************
* @����	: 
* @����	: ����10MS�����ڽ������ 
*********************************************************************************************************
**/
void common_idle_judge(usart_frame_t *com_pointer)
{
	com_pointer->refresh_num++;
	if(com_pointer->refresh_num>=IDLE_NUM)
	{
		com_pointer->refresh_num=IDLE_NUM;
		if(com_pointer->current_num>0)
		{	
			//��������
			common_cache_data(com_pointer->current_num,com_pointer);
			com_pointer->current_num=0;
		}		
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	: ���洮������
*********************************************************************************************************
**/
void common_cache_data(uint16_t buff_long,usart_frame_t *com_pointer)
{
	//�жϻ�������Ƿ���
	if(com_pointer->data_pointer<(CACHE_BUFF_LONGTH-1))
	{
		//��������
		memcpy(com_pointer->data_cache[com_pointer->data_pointer],com_pointer->receive_buff,sizeof(com_pointer->data_cache[0]));		
		//�������ݳ���
		com_pointer->data_cache_long[com_pointer->data_pointer]=buff_long;
		//������ż�һ
		com_pointer->data_pointer++;		
	}
}

/**
*********************************************************************************************************
* @����	: 
* @����	: �������ݽ�����ִ�����β��޷��غ��� 
*********************************************************************************************************
**/
void data_analysis_handle(usart_frame_t* com_pointer,fun_void  common_analysis_handle)
{
	if(com_pointer->data_pointer>0)
	{
			
		//��ս���BUFF
		memset(com_pointer->analysis_buff,0,sizeof(com_pointer->analysis_buff));
		//�ӻ����п������ݵ�����BUFF
		memcpy(com_pointer->analysis_buff,com_pointer->data_cache[com_pointer->data_pointer-1],sizeof(com_pointer->analysis_buff));
		//�������ݳ���
		com_pointer->analysis_long= com_pointer->data_cache_long[com_pointer->data_pointer-1];
		//���ݽ���		
		(*common_analysis_handle)();
		// ��ջ���
		memset(com_pointer->data_cache[com_pointer->data_pointer-1],0,sizeof(com_pointer->data_cache[0]));	
		com_pointer->data_cache_long[com_pointer->data_pointer-1]=0;

		com_pointer->data_pointer--;				
	}		
}
/**
*********************************************************************************************************
* @����	: 
* @����	: ���ڽ����жϻص����� 
*********************************************************************************************************
**/
void UsartRx3IrqCallback(void)
{
	/*insied*/
	uint16_t  data;	
	static portBASE_TYPE xHigherPriorityTaskWoken;	
	/*�ж�����*/
	__disable_irq();	
	xHigherPriorityTaskWoken = pdFALSE;
	/*�������� ���뻺��*/
    data = USART_RecData(USART_CH);
	common_receive_refresh(data,&usart3_commnuication_frame);
	/*�ж�ʹ��*/	
	__enable_irq();
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );	
}
/**
*********************************************************************************************************
* @����	: 
* @����	: ���ڽ����жϻص����� 
*********************************************************************************************************
**/
void UsartRx4IrqCallback(void)
{
	/*outsied*/
	uint16_t  data;	
	static portBASE_TYPE xHigherPriorityTaskWoken;	
	/*�ж�����*/
	__disable_irq();	
	xHigherPriorityTaskWoken = pdFALSE;
	/*�������� ���뻺��*/
    data = USART_RecData(USART_CH2);
	common_receive_refresh(data,&usart4_commnuication_frame);
	/*�ж�ʹ��*/	
	__enable_irq();
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );	
}
/**
*********************************************************************************************************
* @����	: 
* @����	: 
*********************************************************************************************************
**/
void Timer6_OverFlow_CallBack(void)
{
	common_idle_judge(&usart3_commnuication_frame);
	common_idle_judge(&usart4_commnuication_frame);
}

/***********************************************END*****************************************************/


