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
#include "debug_bsp.h"
#include "broker_task.h"
#include "user_type.h"
#include "button_business.h"
/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
#define STATE_NUM 2

/*
*********************************************************************************************************
Typedef
*********************************************************************************************************
*/
typedef struct  parameter_def
{
	u8  status;	  		/*����״̬*/	
}parameter_t;

/*
*********************************************************************************************************
Variables
*********************************************************************************************************
*/
static data_stream_t current_stream_int;

business_information_t  button_business=
{
	.release_name = "button",
	.subscribe_name = "null",		
	.release_data.serial =0,
};
u8 button_buff[10]="key:";
static u8 current_task_status = TASK_INIT;
/*����������ӿ�*/
interface_template_t button_driver_internface;
/*��ǰ�ӿڲ���ָ��*/
static interface_template_t* current_operation = &button_driver_internface;
/*��ǰ��������ָ��*/
static parameter_t uesr_parameter;
static u8* current_parameter  = (u8*)(&uesr_parameter);
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/

static u8 task_init(void);
static u8 task_run(void);

static const callback_u8_void task_cllback[STATE_NUM] = 
{
	task_init,
	task_run,
};

/**
*********************************************************************************************************
* @����	: 
* @����	: 
* @����	: 
* @����	: 
*********************************************************************************************************
**/
void button_task(void *pvParameters)
{
	/*�ص��������Χ*/
	u8 parameter_limit=2;
	while(1)
	{
		/*����ָ��ִ�в���*/
		if(current_task_status<parameter_limit)
		{
			task_cllback[current_task_status]();			
		}
	}		
}
/**
*********************************************************************************************************
* @����	: 
* @����	: 
* @����	: 
* @����	: 
*********************************************************************************************************
**/
static u8 task_init(void)
{
	current_operation->initialization(&current_stream_int, current_parameter);
	/*�л�״̬*/
	current_task_status = TASK_RUN;
	return 0;
}
/**
*********************************************************************************************************
* @����	: 
* @����	: 
* @����	: 
* @����	: 
*********************************************************************************************************
**/
static u8 task_run(void)
{
	static uint16_t clk=0;
	static uint16_t button_business_num=1;
	clk++;
	
	if( (clk %3) == 0)
	{
		current_stream_int.cmd = 1;
		current_stream_int.buff[0]=0xff;
		current_operation->read(&current_stream_int, current_parameter);
		if(current_stream_int.buff[0] != 0xff)
		{
			button_business.release_data.buff[0] = current_stream_int.buff[0];  /*ͨ�� 1~7*/
			button_business.release_data.buff[1] = current_stream_int.buff[1];  /*������*/
			/*������Ϣ ��д���ݺ�д���к�*/
			button_business.release_data.serial = button_business_num++;
		}		
	}
	current_operation->run(&current_stream_int, current_parameter);
	vTaskDelay(1);
	return 0;
}

/***********************************************END*****************************************************/

