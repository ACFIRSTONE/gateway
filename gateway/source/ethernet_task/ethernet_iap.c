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
#include "string.h"
#include "stdlib.h"
#include "debug_bsp.h"
#include "ethernet_data_handle.h"
#include "ethernet_operation.h"

#include "data_task.h"
///////////////////////////////////////////////����//////////////////////////////////////////
#include "wizchip_conf.h" //������Ҫ��ͷ�ļ�
#include "socket.h"       //������Ҫ��ͷ�ļ�
#include "dhcp.h"         //������Ҫ��ͷ�ļ�
#include "spi.h"          //������Ҫ��ͷ�ļ�
#include "ethernet_task.h"
#include "broker_task.h"
#include "user_type.h"

#include "data_task.h"

#include "logic_control_task.h"
/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
#define RECCEIVE_ASC_NUM 100
#define CODE_ADD 5
#define TCP_ANALYSIS_MAX 4
#define SEND_NUM 120
#define IAP_SYNCH 		0XF1
#define IAP_SYNCH_ACK 	0XF2

#define IAP_START 		0XF3
#define IAP_START_ACK 	0XF4

#define IAP_SEND 		0XF5
#define IAP_SEND_ACK 	0XF6

#define IAP_FINISH 		0XF7
#define IAP_FINISH_ACK 	0XF8

#define IAP_ERR_ACK 	0XFE
#define SECTOR_SIZE  (1024*8)

#define FLASH_SECTOR_SIZE           0x2000ul
#define FLASH_SIZE                  (64u * FLASH_SECTOR_SIZE)
#define RAM_SIZE                    0x2F000ul
#define FLASH_BASE            ((uint32_t)0x00000000) /*!< FLASH base address in the alias region */
#define SRAM_BASE             ((uint32_t)0x1FFF8000) /*!< SRAM base address in the alias region */
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
static u8 write_num = 0;
static u8 start_num = 0;
static u8 iap_receive_flag = 0;
static u8 iap_receive_num = 0;
static u8 iap_finish_num = 0;

void clear_iap_flag(void);
/**
*********************************************************************************************************
* @����	: 
* @����	: ��־λ���
* @����	: 
* @����	: 
*********************************************************************************************************
**/
u8 app_area_detection(void)
{
	u8 ret = 0;
	uint32_t *u32Addr=(uint32_t*) (FLASH_IAP_MARK_ADRR );
	if((0x1234 == u32Addr[0]) && (0x5678 == u32Addr[1]) && (0x3141 == u32Addr[3]) && (0x5926 == u32Addr[4]))
	{
		if(u32Addr[2] == 0xa1a1)
			ret = 1;
		else if(u32Addr[2] == 0xa2a2)
			ret = 2;
	}
	return ret;
}
/**
*********************************************************************************************************
* @����	: 
* @����	: tcp iap �û���־
*********************************************************************************************************
**/
void boot_mark_set(void)
{
	uint32_t u32Addr;	
	u32 user_sequence[5] = {0x1234,0x5678,0xa1a1,0x3141,0x5926};

    /* Unlock EFM.*/
    EFM_Unlock();
    /* Enable flash.*/
    EFM_FlashCmd(Enable);
    /* Wait flash ready. */
    while(Set != EFM_GetFlagStatus(EFM_FLAG_RDY));
    /* Erase sector  */
    EFM_SectorErase(FLASH_IAP_MARK_ADRR);
	u32Addr = FLASH_IAP_MARK_ADRR;
    for(u8 i = 0u; i < 5; i++)
    {
        EFM_SingleProgram(u32Addr,user_sequence[i]);
        u32Addr += 4u;
    }	
    /* Lock EFM. */
    EFM_Lock();
	
}

/**
*********************************************************************************************************
* @����	: 
* @����	: tcp iap ����û���־
*********************************************************************************************************
**/
void boot_mark_clear(void)
{
    /* Unlock EFM.*/
    EFM_Unlock();
    /* Enable flash.*/
    EFM_FlashCmd(Enable);
    /* Wait flash ready. */
    while(Set != EFM_GetFlagStatus(EFM_FLAG_RDY));
    /* Erase sector  */
    EFM_SectorErase(FLASH_IAP_MARK_ADRR);	
    /* Lock EFM. */
    EFM_Lock();
	
}
/**
*********************************************************************************************************
* @����	: 
* @����	: 
*********************************************************************************************************
**/
u8 boot_mark_detection(void)
{
	u32 user_sequence[5] = {0x1234,0x5678,0xa1a1,0x3141,0x5926};	
	u8 ret = 0;
	uint32_t *u32Addr=(uint32_t*) (FLASH_IAP_MARK_ADRR );
	if((user_sequence[0] == u32Addr[0]) && (user_sequence[1] == u32Addr[1]) && (user_sequence[2] == u32Addr[2])
		&& (user_sequence[3] == u32Addr[3]) && (user_sequence[4] == u32Addr[4]))
	{
		ret = 1;
	}
	return ret;
}

/**
*********************************************************************************************************
* @����	: 
* @����	: tcp iap ����Ӧ��
*********************************************************************************************************
**/
void tcp_iap_reply(uint8_t cmd)
{
	u8 send_buff[10];
	u8 length = 0;
	
	send_buff[length++] = 0X7E;
	send_buff[length++] = 0;	
	send_buff[length++] = 2;
	send_buff[length++] = cmd;
	send_buff[length++] = 0;
	send_buff[length++] = 0X7D;
	
	send(SOCK_LOCAL, send_buff, length);	
}
/**
*********************************************************************************************************
* @����	: 
* @����	:iap��������
*********************************************************************************************************
**/
static u32 iap_flash_sector = 0;
static u32 iap_flash_add = 0;

void iap_receive(uint8_t* reve_buff,int16_t reve_length ,uint8_t *package_num)
{
	int16_t iap_length;
	if(reve_length > 8)
	{
		debug_printf("дFLAHS��ַ%x\n",FLASH_CACHE_ADRR+iap_flash_add );/*��ӡ��Ϣ*/			
		iap_length =  reve_length - 9;
		/*�жϵ�ַ ����FLASH*/
		if(iap_flash_add >= iap_flash_sector*SECTOR_SIZE)
		{
			flash_erase(FLASH_CACHE_ADRR+iap_flash_add);
			iap_flash_sector++;
		}
		/*дFLASH ��ַ����������*/
		flash_write(FLASH_CACHE_ADRR + iap_flash_add ,&reve_buff[7] , iap_length);
		iap_flash_add += iap_length;
		
		debug_printf("�յ���%d�����ݣ��յ�%d�ֽ�\n",(*package_num), iap_length );/*��ӡ��Ϣ*/	
		vTaskDelay(10);/*��ʱ ϵͳ�л�����*/
		write_num++;
		tcp_iap_reply(IAP_SEND_ACK);		
	}
	else
		debug_printf("��������С��8�ֽ��ֽ�\n");/*��ӡ��Ϣ*/
}
/**
*********************************************************************************************************
* @����	: 
* @����	:iap��� 
*********************************************************************************************************
**/
void iap_finish(void)
{
	/* mark��־λ  */
	boot_mark_set();	
	debug_printf("iap���\n");/*��ӡ��Ϣ*/
	vTaskDelay(5);
	tcp_iap_reply(IAP_FINISH_ACK);
	NVIC_SystemReset();	 /*������BOOT�л�������ģʽ  ��ʹ�ó��Һ��û�����һ��IAP���̣�*/
}

/**
*********************************************************************************************************
* @����	: 
* @����	: ������ʱ
*********************************************************************************************************
**/

u8 iap_time_handle(u8 cmd)
{
	static u16 time_num = 0;
	if(cmd == 0)
	{
		time_num++;
	}
	else if(cmd == 1)
	{
		time_num = 0;
	}

	if(time_num > 5000)
	{
		time_num = 0;
		if(iap_receive_flag == 1)
		{
			debug_printf("iap��ʱ\n");/*��ӡ��Ϣ*/
			clear_iap_flag();
		}
	}
	return 0;
}
/**
*********************************************************************************************************
* @����	: 
* @����	: ���־λ
*********************************************************************************************************
**/

void clear_iap_flag(void)
{
		/*��λLASFH����*/
	iap_receive_flag = 0;
	iap_receive_num = 0;
	iap_flash_sector = 0;
	iap_flash_add = 0;
	start_num = 0;
	iap_finish_num = 0;	
	debug_printf("iap���־λ\n");/*��ӡ��Ϣ*/
}
/**
*********************************************************************************************************
* @����	: 
* @����	: tcp iap ����
*********************************************************************************************************
**/
void tcp_iap_handle(uint8_t* reve_buff,int16_t reve_length)
{
	/*��¼�ְ��� ����ʱ������*/
	if((iap_receive_flag == 1)&&(iap_finish_num == 0))
	{
		iap_receive_num++;
	}	
	if((reve_buff[0] == 0x7E) && (reve_buff[1] == 0x00)&& (reve_buff[2] == 0x02))/*�жϰ�ͷ*/
	{

		if(reve_buff[3] == IAP_START)/*��������*/
		{
			start_num++;
			if((start_num > 3 )&& (iap_receive_flag == 0))
			{
				debug_printf("iap����\n");/*��ӡ��Ϣ*/
				tcp_iap_reply(IAP_START_ACK);
				iap_time_handle(1);
				iap_receive_flag = 1;
				/*��λLASFH����*/
				iap_receive_num = 0;
				iap_flash_sector = 0;
				iap_flash_add = 0;
				start_num = 0;
				iap_finish_num = 0;
				
			}	
		}
		else if(reve_buff[3] == IAP_SEND)/*��������*/
		{
			if(iap_receive_flag == 1)
			{
				if(reve_buff[4] == iap_receive_num)
				{			
					iap_receive(reve_buff, reve_length ,&iap_receive_num);	
					iap_time_handle(1);
				}
				else
				{
					tcp_iap_reply(IAP_ERR_ACK);
					clear_iap_flag();
				}
			}
		}		
		else if(reve_buff[3] == IAP_FINISH)/*��������*/
		{
			iap_finish_num++;
			if(iap_receive_flag == 1)
			{
				if((iap_receive_num == (reve_buff[4]+1)) &&(iap_receive_num > 10))
				{
					if(iap_finish_num > 3)
					{
						iap_finish();
					}
				}
				else
				{
					tcp_iap_reply(IAP_ERR_ACK);
					clear_iap_flag();
				}
			}
		}	
	}
}

/**
*********************************************************************************************************
* @����	: 
* @����	: ���ƻ���̼����ݵ��û��� 
*********************************************************************************************************
**/
void copy_data_to_app(void)
{
	u8 *app_cache=(u8*) (FLASH_IAP_MARK_ADRR );
	for(u8 i = 0 ; i < 11 ; i ++)
	{
		flash_erase(FLASH_USER_FITMWAVE_ADRR+i*0x2000);
		app_cache =(u8*) (FLASH_CACHE_ADRR +i*0x2000);
		flash_write( FLASH_USER_FITMWAVE_ADRR+i*0x2000  , app_cache , 0x2000);
		/*ι��*/		
		WDT_RefreshCounter();
		debug_printf("������%d����\n", i);/*��ӡ��Ϣ*/
	}
	/*������*/		
	boot_mark_clear();
}
/**
*********************************************************************************************************
* @����	: 
* @����	: �ر�����
*********************************************************************************************************
**/
void boot_close_peripheral(void)
{
	//SysTick_Suspend();
	//rs485_usart_close();
}

/**
*********************************************************************************************************
* @����	: 
* @����	: IAP��ת
*********************************************************************************************************
**/
void iap_jump_to_app(void)
{
    uint32_t stack_top = *((__IO uint32_t *)FLASH_USER_FITMWAVE_ADRR);
	func_ptr_t jump_to_application;
	uint32_t app_jump_add;
    /* �ж�ջ����ַ��Ч�� */
    if ((stack_top > SRAM_BASE) && (stack_top <= (SRAM_BASE + RAM_SIZE)))
    {
		 /* �ر�����*/
		boot_close_peripheral();
		 /* ������ת���û�����λ�ж���� */
        app_jump_add = *(__IO uint32_t *)(FLASH_USER_FITMWAVE_ADRR + 4);
		/* ����ַָ��ǿ��ת���ɺ���ָ��*/
        jump_to_application = (func_ptr_t)app_jump_add;
        /* ����ջ */
        __set_MSP(*(__IO uint32_t *)FLASH_USER_FITMWAVE_ADRR);
		/* ��ת*/
        jump_to_application();
    }
}

/***********************************************END*****************************************************/
