/**
*********************************************************************************************************
*                                        multi_switch_control
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


/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/


#define TASK_INIT 	0
#define TASK_RUN 	1
#define TASK_STATE0 	0
#define TASK_STATE1 	1
#define TASK_STATE2 	2
#define TASK_STATE3		3
#define TASK_STATE4 	4
/*�������������*/
#define DATA_STREAM_BUFF_MAX 50

#define TASK_STATE_NUM_1     1
#define TASK_STATE_NUM_2     2
#define TASK_STATE_NUM_3     3
#define TASK_STATE_NUM_4     4



/*
*********************************************************************************************************
Typedef
*********************************************************************************************************
*/
/*��������*/
typedef  int	  int32_t;
typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned int	  uint32_t;
typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef void (*callback_void_void)(void);

typedef u8 (*callback_u8_void)(void);
typedef u8 (*callback_u8_u8)(u8 data);

typedef char int8;

typedef volatile char vint8;

typedef unsigned char uint8;

typedef volatile unsigned char vuint8;

typedef int int16;

typedef unsigned short uint16;

typedef long int32;

typedef unsigned long uint32;

typedef uint8			u_char;		/**< 8-bit value */
typedef uint8 			SOCKET;
typedef uint16			u_short;	/**< 16-bit value */
typedef uint16			u_int;		/**< 16-bit value */
typedef uint32			u_long;		/**< 32-bit value */

/*ͨ������������*/
typedef struct data_stream_def
{		   								
	u8 cmd;
	u8 buff[DATA_STREAM_BUFF_MAX];
}data_stream_t;

/*��������ģ�� */
typedef  u8 (*fun_template_t)(data_stream_t* stream ,u8* parameter );

/*�����ģ��*/
typedef struct interface_template_def
{	
	/*��ȡ���ݲ���*/	
	fun_template_t read;
	/*��ȡ���ݲ���*/
	fun_template_t write;
	/*��ʼ������*/
	fun_template_t initialization;
	/*�жϲ���  ���ڼ��� �ñ�־λ  �����жϺ����� ��ʵʱ��*/
	fun_template_t interrput;
	/*�������в��� */
	fun_template_t  run;
}interface_template_t;

/*��������ģ�� */
typedef  u8 (*fun_bsp_t)(data_stream_t* stream );
/*�����ģ��*/
typedef struct interface_bsp_def
{	
	/*��ȡ���ݲ���*/	
	fun_bsp_t read;
	/*��ȡ���ݲ���*/
	fun_bsp_t write;
	/*��ʼ������*/
	fun_bsp_t initialization;
	/*�жϲ���  ���ڼ��� �ñ�־λ  �����жϺ����� ��ʵʱ��*/
	fun_bsp_t interrput;
	/*�������в��� */
	fun_bsp_t  run;
}interface_bsp_t;
/*
*********************************************************************************************************
Variables
*********************************************************************************************************
*/

/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/

/***********************************************END*****************************************************/


