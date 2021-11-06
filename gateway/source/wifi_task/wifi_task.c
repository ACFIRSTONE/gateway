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
#include "wifi_task.h"
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

/*
*********************************************************************************************************
Variables
*********************************************************************************************************
*/
TaskHandle_t wifi_task_handler;
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/

/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
/* USART channel definition */
#define WIFI_CH                        (M4_USART4)

/* USART baudrate definition */
#define WIFI_BAUDRATE                  (115200)

/* USART RX Port/Pin definition */
#define WIFI_RX_PORT                   (PortB)
#define WIFI_RX_PIN                    (Pin15)
#define WIFI_RX_FUNC                   (Func_Usart4_Rx)

/* USART TX Port/Pin definition */
#define WIFI_TX_PORT                   (PortB)
#define WIFI_TX_PIN                    (Pin14)
#define WIFI_TX_FUNC                   (Func_Usart4_Tx)

/* USART interrupt number  */
#define WIFI_RI_NUM                    (INT_USART4_RI)
#define WIFI_EI_NUM                    (INT_USART4_EI)
#define WIFI_TI_NUM                    (INT_USART4_TI)
#define WIFI_TCI_NUM                   (INT_USART4_TCI)


#define WIFI_RESET_PORT                   (PortA)
#define WIFI_RESET_PIN                    (Pin02)
#define RESET_IO_H    PORT_SetBits(WIFI_RESET_PORT, WIFI_RESET_PIN) 
#define RESET_IO_L    PORT_ResetBits(WIFI_RESET_PORT, WIFI_RESET_PIN) 

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

/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
#include "rs458_analysis.h"
usart_frame_t   wifi_usart2_frame;
u8 wifi_judge_num=0;
void Usart4_RxIrqCallback(void);

/**
 *******************************************************************************
 ** \brief USART RX error irq callback function.
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
static void Usart4_ErrIrqCallback(void)
{
    if (Set == USART_GetStatus(WIFI_CH, UsartFrameErr))
    {
        USART_ClearStatus(WIFI_CH, UsartFrameErr);
    }
    else
    {
    }

    if (Set == USART_GetStatus(WIFI_CH, UsartParityErr))
    {
        USART_ClearStatus(WIFI_CH, UsartParityErr);
    }
    else
    {
    }

    if (Set == USART_GetStatus(WIFI_CH, UsartOverrunErr))
    {
        USART_ClearStatus(WIFI_CH, UsartOverrunErr);
    }
    else
    {
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
void wifi_usart_init(void)
{
    en_result_t enRet = Ok;
    stc_irq_regi_conf_t stcIrqRegiCfg;
    uint32_t u32Fcg1Periph = PWC_FCG1_PERIPH_USART1 | PWC_FCG1_PERIPH_USART2 | \
                             PWC_FCG1_PERIPH_USART3 | PWC_FCG1_PERIPH_USART4;
    const stc_usart_uart_init_t stcInitCfg = {
        UsartIntClkCkNoOutput,
        UsartClkDiv_1,
        UsartDataBits8,
        UsartDataLsbFirst,
        UsartOneStopBit,
        UsartParityNone,
        UsartSamleBit8,
        UsartStartBitFallEdge,
        UsartRtsEnable,
    };
    /* Enable peripheral clock */
    PWC_Fcg1PeriphClockCmd(u32Fcg1Periph, Enable);

    /* Initialize USART IO */
    PORT_SetFunc(WIFI_RX_PORT, WIFI_RX_PIN, WIFI_RX_FUNC, Disable);
    PORT_SetFunc(WIFI_TX_PORT, WIFI_TX_PIN, WIFI_TX_FUNC, Disable);

    /* Initialize UART */
    enRet = USART_UART_Init(WIFI_CH, &stcInitCfg);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }
    /* Set baudrate */
    enRet = USART_SetBaudrate(WIFI_CH, WIFI_BAUDRATE);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }

    /* Set USART RX IRQ */
    stcIrqRegiCfg.enIRQn = Int001_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart4_RxIrqCallback;
    stcIrqRegiCfg.enIntSrc = WIFI_RI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    /* Set USART RX error IRQ */
    stcIrqRegiCfg.enIRQn = Int002_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart4_ErrIrqCallback;
    stcIrqRegiCfg.enIntSrc = WIFI_EI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
	
    /*Enable RX && RX interupt function*/
    USART_FuncCmd(WIFI_CH, UsartRx, Enable);
    USART_FuncCmd(WIFI_CH, UsartRxInt, Enable);
	
	USART_FuncCmd(WIFI_CH, UsartTxAndTxEmptyInt, Enable);


	//////RESET
    stc_port_init_t stcPortInit;
    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;
    PORT_Init(WIFI_RESET_PORT, WIFI_RESET_PIN, &stcPortInit);
	
	
}

extern char Connect_flag;  //�ⲿ����������ͬ����������״̬  0����û�����ӷ�����  1�������Ϸ�������

#define USART2_RXBUFF_SIZE   1024              //���崮��2 ���ջ�������С 1024�ֽ�

extern char Usart2_RxCompleted ;               //�ⲿ�����������ļ����Ե��øñ���
extern unsigned int Usart2_RxCounter;          //�ⲿ�����������ļ����Ե��øñ���
extern char Usart2_RxBuff[USART2_RXBUFF_SIZE]; //�ⲿ�����������ļ����Ե��øñ���


char Usart2_RxCompleted = 0;            //����һ������ 0����ʾ����δ��� 1����ʾ������� 
unsigned int Usart2_RxCounter = 0;      //����һ����������¼����2�ܹ������˶����ֽڵ�����
char Usart2_RxBuff[USART2_RXBUFF_SIZE]; //����һ�����飬���ڱ��洮��2���յ�������   

#define WiFi_printf       u2_printf           //����2���� WiFi
#define WiFi_RxCounter    Usart2_RxCounter    //����2���� WiFi
#define WiFi_RX_BUF       Usart2_RxBuff       //����2���� WiFi
#define WiFi_RXBUFF_SIZE  USART2_RXBUFF_SIZE  //����2���� WiFi



#define SSID   "tofan"                     //·����SSID����
#define PASS   "4006352166"                 //·��������

char *ServerIP = "47.106.234.88";           //��ŷ�����IP��������
int  ServerPort = 5188;                      //��ŷ������Ķ˿ں���


void WiFi_ResetIO_Init(void);
char WiFi_SendCmd(char *cmd, int timeout);
char WiFi_Reset(int timeout);
char WiFi_JoinAP(int timeout);
char WiFi_Connect_Server(int timeout);
char WiFi_Smartconfig(int timeout);
char WiFi_WaitAP(int timeout);
char WiFi_GetIP(u16 timeout);
char WiFi_Get_LinkSta(void);
char WiFi_Get_Data(char *data, char *len, char *id);
char WiFi_SendData(char id, char *databuff, int data_len, int timeout);
char WiFi_Connect_Server(int timeout);
char WiFi_ConnectServer(void);
char wifi_mode = 0;     //����ģʽ 0��SSID������д�ڳ�����   1��Smartconfig��ʽ��APP����
char Connect_flag;      //ͬ����������״̬  0����û�����ӷ�����  1�������Ϸ�������


#include "debug_bsp.h"
#include <stdarg.h>
__align(8) char USART2_TxBuff[200];  

void u2_printf(char* fmt,...) 
{  
	unsigned int i,length;
	
	va_list ap;
	va_start(ap,fmt);
	vsprintf(USART2_TxBuff,fmt,ap);
	va_end(ap);	
	
	length=strlen((const char*)USART2_TxBuff);

	for(uint8_t i = 0; i < length; i++)
	{
		while (Reset == USART_GetStatus(WIFI_CH, UsartTxEmpty));
		USART_SendData(WIFI_CH, USART2_TxBuff[i]);
		while (Reset == USART_GetStatus(WIFI_CH, UsartTxComplete)); 
		USART_ClearStatus(WIFI_CH, UsartTxComplete);
	}		
}
#define DEBUG_TXBUFF_SIZE   256   
extern char debug_buff[DEBUG_TXBUFF_SIZE]; 
void u1_printf(char* fmt,...) 
{  
	unsigned int i,length;
	/*�ɱ��������*/
	va_list ap;
	va_start(ap,fmt);
	vsprintf(debug_buff,fmt,ap);
	va_end(ap);	
	/*�ɱ��������*/
	length=strlen((const char*)debug_buff);
	debug_output((unsigned char*)debug_buff,length);	
}
/*-------------------------------------------------*/
/*��������WiFi��������ָ��                         */
/*��  ����cmd��ָ��                                */
/*��  ����timeout����ʱʱ�䣨100ms�ı�����         */
/*����ֵ��0����ȷ   ����������                     */
/*-------------------------------------------------*/
char WiFi_SendCmd(char *cmd, int timeout)
{
	WiFi_RxCounter=0;                           //WiFi������������������                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //���WiFi���ջ����� 
	WiFi_printf("%s\r\n",cmd);                  //����ָ��
	while(timeout--){                           //�ȴ���ʱʱ�䵽0
		vTaskDelay(100);                          //��ʱ100ms
		if(strstr(WiFi_RX_BUF,"OK"))            //������յ�OK��ʾָ��ɹ�
			break;       						//��������whileѭ��
		debug_printf("%d ",timeout);               //����������ڵĳ�ʱʱ��
	}
	debug_printf("\r\n");                          //���������Ϣ
	if(timeout<=0)return 1;                     //���timeout<=0��˵����ʱʱ�䵽�ˣ�Ҳû���յ�OK������1
	else return 0;		         				//��֮����ʾ��ȷ��˵���յ�OK��ͨ��break��������while
}
/*-------------------------------------------------*/
/*��������WiFi��λ                                 */
/*��  ����timeout����ʱʱ�䣨100ms�ı�����         */
/*����ֵ��0����ȷ   ����������                     */
/*-------------------------------------------------*/
char WiFi_Reset(int timeout)
{
	RESET_IO_L;                                    //��λIO���͵�ƽ
	vTaskDelay(500);                                  //��ʱ500ms
	RESET_IO_H;                                    //��λIO���ߵ�ƽ	
	while(timeout--){                               //�ȴ���ʱʱ�䵽0
		vTaskDelay(100);                              //��ʱ100ms
		if(strstr(WiFi_RX_BUF,"ready"))             //������յ�ready��ʾ��λ�ɹ�
			break;       						    //��������whileѭ��
		debug_printf("%d ",timeout);                   //����������ڵĳ�ʱʱ��
	}
	debug_printf("\r\n");                              //���������Ϣ
	if(timeout<=0)return 1;                         //���timeout<=0��˵����ʱʱ�䵽�ˣ�Ҳû���յ�ready������1
	else return 0;		         				    //��֮����ʾ��ȷ��˵���յ�ready��ͨ��break��������while
}
/*-------------------------------------------------*/
/*��������WiFi����·����ָ��                       */
/*��  ����timeout����ʱʱ�䣨1s�ı�����            */
/*����ֵ��0����ȷ   ����������                     */
/*-------------------------------------------------*/
char WiFi_JoinAP(int timeout)
{		
	WiFi_RxCounter=0;                               //WiFi������������������                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //���WiFi���ջ����� 
	WiFi_printf("AT+CWJAP=\"%s\",\"%s\"\r\n",SSID,PASS); //����ָ��	
	while(timeout--){                               //�ȴ���ʱʱ�䵽0
		vTaskDelay(100);                             //��ʱ1s
		//if(strstr(WiFi_RX_BUF,"WIFI GOT IP\r\n\r\nOK")) //������յ�WIFI GOT IP��ʾ�ɹ�
		if(strstr(WiFi_RX_BUF,"WIFI GOT IP")) //������յ�WIFI GOT IP��ʾ�ɹ�
		{
			char delay=50;
			while(delay--)
			{ 
				vTaskDelay(100);
				if(strstr(WiFi_RX_BUF,"OK")) //������յ�WIFI GOT IP��ʾ�ɹ�
					return 0;
			}
				
		}			
			//break;       						    //��������whileѭ��
		debug_printf("%d ",timeout);                   //����������ڵĳ�ʱʱ��
	}
	debug_printf("\r\n");                              //���������Ϣ
	if(timeout<=0)return 1;                         //���timeout<=0��˵����ʱʱ�䵽�ˣ�Ҳû���յ�WIFI GOT IP������1
	return 0;                                       //��ȷ������0
}
/*-------------------------------------------------*/
/*��������WiFi_Smartconfig                         */
/*��  ����timeout����ʱʱ�䣨1s�ı�����            */
/*����ֵ��0����ȷ   ����������                     */
/*-------------------------------------------------*/
char WiFi_Smartconfig(int timeout)
{	
	WiFi_RxCounter=0;                           //WiFi������������������                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //���WiFi���ջ�����     
	while(timeout--){                           //�ȴ���ʱʱ�䵽0
		vTaskDelay(1000);                         //��ʱ1s
		if(strstr(WiFi_RX_BUF,"connected"))     //������ڽ��ܵ�connected��ʾ�ɹ�
			break;                              //����whileѭ��  
		debug_printf("%d ",timeout);               //����������ڵĳ�ʱʱ��  
	}	
	debug_printf("\r\n");                          //���������Ϣ
	if(timeout<=0)return 1;                     //��ʱ���󣬷���1
	return 0;                                   //��ȷ����0
}
/*-------------------------------------------------*/
/*���������ȴ�����·����                           */
/*��  ����timeout����ʱʱ�䣨1s�ı�����            */
/*����ֵ��0����ȷ   ����������                     */
/*-------------------------------------------------*/
char WiFi_WaitAP(int timeout)
{		
	while(timeout--){                               //�ȴ���ʱʱ�䵽0
		vTaskDelay(1000);                             //��ʱ1s
		if(strstr(WiFi_RX_BUF,"WIFI GOT IP"))       //������յ�WIFI GOT IP��ʾ�ɹ�
			break;       						    //��������whileѭ��
		debug_printf("%d ",timeout);                   //����������ڵĳ�ʱʱ��
	}
	debug_printf("\r\n");                              //���������Ϣ
	if(timeout<=0)return 1;                         //���timeout<=0��˵����ʱʱ�䵽�ˣ�Ҳû���յ�WIFI GOT IP������1
	return 0;                                       //��ȷ������0
}
/*-------------------------------------------------*/
/*���������ȴ�����wifi����ȡIP��ַ                 */
/*��  ����ip������IP������                         */
/*��  ����timeout����ʱʱ�䣨100ms�ı�����         */
/*����ֵ��0����ȷ   ����������                     */
/*-------------------------------------------------*/
char ip_show_buff[20];
char WiFi_GetIP(u16 timeout)
{
	char *presult1,*presult2;
	char ip[50];
		                              	
	WiFi_RxCounter=0;                               //WiFi������������������ 
	
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //���WiFi���ջ����� 
	vTaskDelay(1000);
	WiFi_printf("AT+CIFSR\r\n");                    //����ָ��	
	while(timeout--){                               //�ȴ���ʱʱ�䵽0
		vTaskDelay(100);                              //��ʱ100ms
		if(strstr(WiFi_RX_BUF,"OK"))                //������յ�OK��ʾ�ɹ�
			break;       						    //��������whileѭ��
		debug_printf("%d ",timeout);                   //����������ڵĳ�ʱʱ��
	}
	debug_printf("\r\n");                              //���������Ϣ
	if(timeout<=0)
		return 1;                         //���timeout<=0��˵����ʱʱ�䵽�ˣ�Ҳû���յ�OK������1
	else
	{
		presult1 = strstr(WiFi_RX_BUF,"\"");
		if( presult1 != NULL ){
			presult2 = strstr(presult1+1,"\"");
			if( presult2 != NULL ){
				memcpy(ip,presult1+1,presult2-presult1-1);
				memcpy(ip_show_buff,ip,20);
				debug_printf("ESP8266��IP��ַ��%s\r\n",ip);     //������ʾIP��ַ
				return 0;    //��ȷ����0
			}else 
			return 2;  //δ�յ�Ԥ������
		}else 
		return 3;      //δ�յ�Ԥ������	
	}
}
/*-------------------------------------------------*/
/*����������ȡ����״̬                             */
/*��  ������                                       */
/*����ֵ������״̬                                 */
/*        0����״̬                                */
/*        1���пͻ��˽���                          */
/*        2���пͻ��˶Ͽ�                          */
/*-------------------------------------------------*/
char WiFi_Get_LinkSta(void)
{
	char id_temp[10]={0};    //�����������ID
	char sta_temp[10]={0};   //�����������״̬
	
	if(strstr(WiFi_RX_BUF,"CONNECT")){                 //������ܵ�CONNECT��ʾ�пͻ�������	
		sscanf(WiFi_RX_BUF,"%[^,],%[^,]",id_temp,sta_temp);
		debug_printf("�пͻ��˽��룬ID=%s\r\n",id_temp);  //������ʾ��Ϣ
		WiFi_RxCounter=0;                              //WiFi������������������                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);        //���WiFi���ջ�����     
		return 1;                                      //�пͻ��˽���
	}else if(strstr(WiFi_RX_BUF,"CLOSED")){            //������ܵ�CLOSED��ʾ�����ӶϿ�	
		sscanf(WiFi_RX_BUF,"%[^,],%[^,]",id_temp,sta_temp);
		debug_printf("�пͻ��˶Ͽ���ID=%s\r\n",id_temp);        //������ʾ��Ϣ
		WiFi_RxCounter=0;                                    //WiFi������������������                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);              //���WiFi���ջ�����     
		return 2;                                            //�пͻ��˶Ͽ�
	}else return 0;                                          //��״̬�ı�	
}
/*-------------------------------------------------*/
/*����������ȡ�ͻ�������                           */
/*        ����س����з�\r\n\r\n��Ϊ���ݵĽ�����   */
/*��  ����data�����ݻ�����                         */
/*��  ����len�� ������                             */
/*��  ����id��  �������ݵĿͻ��˵�����ID           */
/*����ֵ������״̬                                 */
/*        0��������                                */
/*        1��������                                */
/*-------------------------------------------------*/
char WiFi_Get_Data(char *data, char *len, char *id)
{
	char temp[10]={0};      //������
	char *presult;

	if(strstr(WiFi_RX_BUF,"\r\n\r\n")){                     //�������ŵĻس�������Ϊ���ݵĽ�����
		sscanf(WiFi_RX_BUF,"%[^,],%[^,],%[^:]",temp,id,len);//��ȡ�������ݣ���Ҫ��id�����ݳ���	
		presult = strstr(WiFi_RX_BUF,":");                  //����ð�š�ð�ź��������
		if( presult != NULL )                               //�ҵ�ð��
			sprintf((char *)data,"%s",(presult+1));         //ð�ź�����ݣ����Ƶ�data
		WiFi_RxCounter=0;                                   //WiFi������������������                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);             //���WiFi���ջ�����    
		return 1;                                           //�����ݵ���
	} else return 0;                                        //�����ݵ���
}
/*-------------------------------------------------*/
/*����������������������                           */
/*��  ����databuff�����ݻ�����<2048                */
/*��  ����data_len�����ݳ���                       */
/*��  ����id��      �ͻ��˵�����ID                 */
/*��  ����timeout�� ��ʱʱ�䣨10ms�ı�����         */
/*����ֵ������ֵ                                   */
/*        0���޴���                                */
/*        1���ȴ��������ݳ�ʱ                      */
/*        2�����ӶϿ���                            */
/*        3���������ݳ�ʱ                          */
/*-------------------------------------------------*/
char WiFi_SendData(char id, char *databuff, int data_len, int timeout)
{    
	int i;
	
	WiFi_RxCounter=0;                                 //WiFi������������������                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);           //���WiFi���ջ����� 
	WiFi_printf("AT+CIPSEND=%d,%d\r\n",id,data_len);  //����ָ��	
    while(timeout--){                                 //�ȴ���ʱ���	
		vTaskDelay(10);                                 //��ʱ10ms
		if(strstr(WiFi_RX_BUF,">"))                   //������յ�>��ʾ�ɹ�
			break;       						      //��������whileѭ��
		debug_printf("%d ",timeout);                     //����������ڵĳ�ʱʱ��
	}
	if(timeout<=0)return 1;                                   //��ʱ���󣬷���1
	else{                                                     //û��ʱ����ȷ       	
		WiFi_RxCounter=0;                                     //WiFi������������������                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);               //���WiFi���ջ����� 	
		for(i=0;i<data_len;i++)WiFi_printf("%c",databuff[i]); //��������	
		while(timeout--){                                     //�ȴ���ʱ���	
			vTaskDelay(10);                                     //��ʱ10ms
			if(strstr(WiFi_RX_BUF,"SEND OK")){                //�������SEND OK����ʾ���ͳɹ�			 
			WiFi_RxCounter=0;                                 //WiFi������������������                        
			memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);           //���WiFi���ջ����� 			
				break;                                        //����whileѭ��
			} 
			if(strstr(WiFi_RX_BUF,"link is not valid")){      //�������link is not valid����ʾ���ӶϿ�			
				WiFi_RxCounter=0;                             //WiFi������������������                        
				memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);       //���WiFi���ջ����� 			
				return 2;                                     //����2
			}
	    }
		if(timeout<=0)return 3;      //��ʱ���󣬷���3
		else return 0;	            //��ȷ������0
	}	
}
/*-------------------------------------------------*/
/*������������TCP��������������͸��ģʽ            */
/*��  ����timeout�� ��ʱʱ�䣨100ms�ı�����        */
/*����ֵ��0����ȷ  ����������                      */
/*-------------------------------------------------*/
char WiFi_Connect_Server(int timeout)
{	
	WiFi_RxCounter=0;                               //WiFi������������������                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //���WiFi���ջ�����   
	WiFi_printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",ServerIP,ServerPort);//�������ӷ�����ָ��
	while(timeout--){                               //�ȴ���ʱ���
		vTaskDelay(100);                              //��ʱ100ms	
		if(strstr(WiFi_RX_BUF ,"CONNECT"))          //������ܵ�CONNECT��ʾ���ӳɹ�
			break;                                  //����whileѭ��
		if(strstr(WiFi_RX_BUF ,"CLOSED"))           //������ܵ�CLOSED��ʾ������δ����
			return 1;                               //������δ��������1
		if(strstr(WiFi_RX_BUF ,"ALREADY CONNECTED"))//������ܵ�ALREADY CONNECTED�Ѿ���������
			return 2;                               //�Ѿ��������ӷ���2
		debug_printf("%d ",timeout);                   //����������ڵĳ�ʱʱ��  
	}
	debug_printf("\r\n");                        //���������Ϣ
	if(timeout<=0)return 3;                   //��ʱ���󣬷���3
	else                                      //���ӳɹ���׼������͸��
	{
		debug_printf("׼������͸��\r\n");                  //������ʾ��Ϣ
		WiFi_RxCounter=0;                               //WiFi������������������                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //���WiFi���ջ�����     
		WiFi_printf("AT+CIPSEND\r\n");                  //���ͽ���͸��ָ��
		while(timeout--){                               //�ȴ���ʱ���
			vTaskDelay(100);                              //��ʱ100ms	
			if(strstr(WiFi_RX_BUF,"\r\nOK\r\n\r\n>"))   //���������ʾ����͸���ɹ�
				break;                          //����whileѭ��
			debug_printf("%d ",timeout);           //����������ڵĳ�ʱʱ��  
		}
		if(timeout<=0)return 4;                 //͸����ʱ���󣬷���4	
	}
	return 0;	                                //�ɹ�����0	
}
/*-------------------------------------------------*/
/*�����������ӷ�����                               */
/*��  ������                                       */
/*����ֵ��0����ȷ   ����������                     */
/*-------------------------------------------------*/
char WiFi_ConnectServer(void)
{	
	char res;
	
	debug_printf("׼����λģ��\r\n");                     //������ʾ����
	if(WiFi_Reset(50)){                                //��λ��100ms��ʱ��λ���ܼ�5s��ʱʱ��
		debug_printf("��λʧ�ܣ�׼������\r\n");           //���ط�0ֵ������if��������ʾ����
		return 1;                                      //����1
	}else u1_printf("��λ�ɹ�\r\n");                   //������ʾ����
	
	debug_printf("׼������STAģʽ\r\n");                  //������ʾ����
	if(WiFi_SendCmd("AT+CWMODE=1",50)){                //����STAģʽ��100ms��ʱ��λ���ܼ�5s��ʱʱ��
		debug_printf("����STAģʽʧ�ܣ�׼������\r\n");    //���ط�0ֵ������if��������ʾ����
		return 2;                                      //����2
	}else debug_printf("����STAģʽ�ɹ�\r\n");            //������ʾ����
	
	if(wifi_mode==0){                                      //�������ģʽ=0��SSID������д�ڳ����� 
		debug_printf("׼��ȡ���Զ�����\r\n");                 //������ʾ����
		if(WiFi_SendCmd("AT+CWAUTOCONN=0",50)){            //ȡ���Զ����ӣ�100ms��ʱ��λ���ܼ�5s��ʱʱ��
			debug_printf("ȡ���Զ�����ʧ�ܣ�׼������\r\n");   //���ط�0ֵ������if��������ʾ����
			return 3;                                      //����3
		}else debug_printf("ȡ���Զ����ӳɹ�\r\n");           //������ʾ����
				
		u1_printf("׼������·����\r\n");                   //������ʾ����	
		if(WiFi_JoinAP(200)){                               //����·����,1s��ʱ��λ���ܼ�30s��ʱʱ��
			u1_printf("����·����ʧ�ܣ�׼������\r\n");     //���ط�0ֵ������if��������ʾ����
			return 4;                                      //����4	
		}else u1_printf("����·�����ɹ�\r\n");             //������ʾ����			
	}else{                                                 //�������ģʽ=1��Smartconfig��ʽ,��APP����
//		if(KEY2_IN_STA==0){                                    //�����ʱK2�ǰ��µ�
//			u1_printf("׼�������Զ�����\r\n");                 //������ʾ����
//			if(WiFi_SendCmd("AT+CWAUTOCONN=1",50)){            //�����Զ����ӣ�100ms��ʱ��λ���ܼ�5s��ʱʱ��
//				u1_printf("�����Զ�����ʧ�ܣ�׼������\r\n");   //���ط�0ֵ������if��������ʾ����
//				return 3;                                      //����3
//			}else u1_printf("�����Զ����ӳɹ�\r\n");           //������ʾ����	
//			
//			u1_printf("׼������Smartconfig\r\n");              //������ʾ����
//			if(WiFi_SendCmd("AT+CWSTARTSMART",50)){            //����Smartconfig��100ms��ʱ��λ���ܼ�5s��ʱʱ��
//				u1_printf("����Smartconfigʧ�ܣ�׼������\r\n");//���ط�0ֵ������if��������ʾ����
//				return 4;                                      //����4
//			}else u1_printf("����Smartconfig�ɹ�\r\n");        //������ʾ����

//			u1_printf("��ʹ��APP�����������\r\n");            //������ʾ����
//			if(WiFi_Smartconfig(60)){                          //APP����������룬1s��ʱ��λ���ܼ�60s��ʱʱ��
//				u1_printf("��������ʧ�ܣ�׼������\r\n");       //���ط�0ֵ������if��������ʾ����
//				return 5;                                      //����5
//			}else u1_printf("��������ɹ�\r\n");               //������ʾ����

//			u1_printf("׼���ر�Smartconfig\r\n");              //������ʾ����
//			if(WiFi_SendCmd("AT+CWSTOPSMART",50)){             //�ر�Smartconfig��100ms��ʱ��λ���ܼ�5s��ʱʱ��
//				u1_printf("�ر�Smartconfigʧ�ܣ�׼������\r\n");//���ط�0ֵ������if��������ʾ����
//				return 6;                                      //����6
//			}else u1_printf("�ر�Smartconfig�ɹ�\r\n");        //������ʾ����
//		}else{                                                 //��֮����ʱK2��û�а���
//			u1_printf("�ȴ�����·����\r\n");                   //������ʾ����	
//			if(WiFi_WaitAP(30)){                               //�ȴ�����·����,1s��ʱ��λ���ܼ�30s��ʱʱ��
//				u1_printf("����·����ʧ�ܣ�׼������\r\n");     //���ط�0ֵ������if��������ʾ����
//				return 7;                                      //����7	
//			}else u1_printf("����·�����ɹ�\r\n");             //������ʾ����					
//		}
	}
	
	u1_printf("׼����ȡIP��ַ\r\n");                   //������ʾ����
	if(WiFi_GetIP(200)){                                //׼����ȡIP��ַ��100ms��ʱ��λ���ܼ�5s��ʱʱ��
		u1_printf("��ȡIP��ַʧ�ܣ�׼������\r\n");     //���ط�0ֵ������if��������ʾ����
		return 10;                                     //����10
	}else u1_printf("��ȡIP��ַ�ɹ�\r\n");             //������ʾ����
	
	u1_printf("׼������͸��\r\n");                     //������ʾ����
	if(WiFi_SendCmd("AT+CIPMODE=1",50)){               //����͸����100ms��ʱ��λ���ܼ�5s��ʱʱ��
		u1_printf("����͸��ʧ�ܣ�׼������\r\n");       //���ط�0ֵ������if��������ʾ����
		return 11;                                     //����11
	}else u1_printf("�ر�͸���ɹ�\r\n");               //������ʾ����
	
	u1_printf("׼���رն�·����\r\n");                 //������ʾ����
	if(WiFi_SendCmd("AT+CIPMUX=0",50)){                //�رն�·���ӣ�100ms��ʱ��λ���ܼ�5s��ʱʱ��
		u1_printf("�رն�·����ʧ�ܣ�׼������\r\n");   //���ط�0ֵ������if��������ʾ����
		return 12;                                     //����12
	}else u1_printf("�رն�·���ӳɹ�\r\n");           //������ʾ����
	
	u1_printf("׼�����ӷ�����\r\n");                   //������ʾ����
	res = WiFi_Connect_Server(100);                    //���ӷ�������100ms��ʱ��λ���ܼ�10s��ʱʱ��
	if(res==1){                						   //����1������if
		u1_printf("������δ������׼������\r\n");       //������ʾ����
		return 13;                                     //����13
	}else if(res==2){                                  //����2������if
		u1_printf("�����Ѿ�����\r\n");                 //������ʾ����
	}else if(res==3){								   //����3������if
		u1_printf("���ӷ�������ʱ��׼������\r\n");     //������ʾ����
		return 14;                                     //����14
	}else if(res==4){								   //����4������if��
		u1_printf("����͸��ʧ��\r\n");                 //������ʾ����
		return 15;                                     //����15
	}	
	u1_printf("���ӷ������ɹ�\r\n");                   //������ʾ����
	return 0;                                          //��ȷ����0	
}
 /**
*********************************************************************************************************
* @����	: 
* @����	: 
* @����	: 
* @����	: 
*********************************************************************************************************
**/
char  Data_buff[2048];     //���ݻ�����
void  completed_judeg(void)
{

		Usart2_RxCompleted = 1;                                       //����2������ɱ�־λ��λ
		memcpy(&Data_buff[2],Usart2_RxBuff,Usart2_RxCounter);         //��������
		Data_buff[0] = WiFi_RxCounter/256;                            //��¼���յ�������		
		Data_buff[1] = WiFi_RxCounter%256;                            //��¼���յ�������
		Data_buff[WiFi_RxCounter+2] = '\0';                           //���������
		WiFi_RxCounter=0;                                             //�������ֵ
        for(u16 i=0;i<USART2_RXBUFF_SIZE;i++)
		{
			if(Usart2_RxBuff[i] == 0)
					Usart2_RxBuff[i]=0x0d;
		}
		
}

void Usart4_RxIrqCallback(void)
{
	
	uint16_t  data;	
	static portBASE_TYPE xHigherPriorityTaskWoken;	
	/*�ж�����*/
	__disable_irq();	
	xHigherPriorityTaskWoken = pdFALSE;
	
		if(Connect_flag==0)
		{                                //���Connect_flag����0����ǰ��û�����ӷ�����������ָ������״̬
			if(WIFI_CH->DR){                                 //����ָ������״̬ʱ������ֵ�ű��浽������	
				Usart2_RxBuff[Usart2_RxCounter]= USART_RecData(WIFI_CH); //���浽������	
				Usart2_RxCounter ++;                        //ÿ����1���ֽڵ����ݣ�Usart2_RxCounter��1����ʾ���յ���������+1 
			}		
		}else
		{		                                        //��֮Connect_flag����1�������Ϸ�������	
			Usart2_RxBuff[Usart2_RxCounter] = USART_RecData(WIFI_CH);   //�ѽ��յ������ݱ��浽Usart2_RxBuff��			
			Usart2_RxCounter ++;         				    //ÿ����1���ֽڵ����ݣ�Usart2_RxCounter��1����ʾ���յ���������+1 
		}
	wifi_judge_num=0;
	/*�ж�ʹ��*/	
	__enable_irq();
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );		
}
/**
*********************************************************************************************************
* @����	: 
* @����	: 
* @����	: 
* @����	: 
*********************************************************************************************************
**/
void wifi_analysis_handle(void)
{
	
	/*��ӡ��Ϣ*/
//	/*ת���ַ���*/			
//	hex_to_ascii(usart3_commnuication_frame.analysis_buff,receive_asc_buff,usart3_commnuication_frame.analysis_long);
//	/*��ӡ���*/
//	debug_printf("TCP �յ����ݣ�%s\n",receive_asc_buff);
//	//debug_printf("comm receive:%s\n",usart3_commnuication_frame.analysis_buff);
//	memset(receive_asc_buff,0,CACHE_BUFF_NUM*2);
	
	
}

/**
*********************************************************************************************************
* @����	: 
* @����	: 
* @����	: 
* @����	: 
*********************************************************************************************************
**/
#include "FreeRTOS.h"
#include "task.h"
/* Define Timer Unit for example */
#define TMR_UNIT            (M4_TMR02)
#define TMR_INI_GCMA        (INT_TMR02_GCMA)

#define ENABLE_TMR0()      (PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM02, Enable))
uint16_t time_test=0;
void Timer0A_CallBack(void)
{
	static portBASE_TYPE xHigherPriorityTaskWoken;	
	/*�ж�����*/
	__disable_irq();	
	xHigherPriorityTaskWoken = pdFALSE;


	wifi_judge_num++;
	if(wifi_judge_num>150)
	{
		wifi_judge_num=0;
		if(WiFi_RxCounter>0)
			completed_judeg();
			
	}  
	time_test++;	
	/*�ж�ʹ��*/	
	__enable_irq();
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );			
}
void time_init(void)
{
    stc_tim0_base_init_t stcTimerCfg;
    stc_irq_regi_conf_t stcIrqRegiConf;
    stc_port_init_t stcPortInit;

    uint32_t u32Pclk1;
    stc_clk_freq_t stcClkTmp;
    uint32_t u32tmp;

    MEM_ZERO_STRUCT(stcTimerCfg);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcPortInit);	
    /* Get pclk1 */
    CLK_GetClockFreq(&stcClkTmp);
    u32Pclk1 = stcClkTmp.pclk1Freq;

    /* Enable XTAL32 */
    CLK_Xtal32Cmd(Enable);

    /* Timer0 peripheral enable */
    ENABLE_TMR0();
    /*config register for channel A */
    stcTimerCfg.Tim0_CounterMode = Tim0_Async;
    stcTimerCfg.Tim0_AsyncClockSource = Tim0_XTAL32;
    stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv4;
    stcTimerCfg.Tim0_CmpValue = (uint16_t)(32/4 - 1);
    TIMER0_BaseInit(TMR_UNIT,Tim0_ChannelA,&stcTimerCfg);


    /* Enable channel A interrupt */
    TIMER0_IntCmd(TMR_UNIT,Tim0_ChannelA,Enable);
    /* Register TMR_INI_GCMA Int to Vect.No.001 */
    stcIrqRegiConf.enIRQn = Int003_IRQn;
    /* Select I2C Error or Event interrupt function */
    stcIrqRegiConf.enIntSrc = TMR_INI_GCMA;
    /* Callback function */
    stcIrqRegiConf.pfnCallback =&Timer0A_CallBack;
    /* Registration IRQ */
    enIrqRegistration(&stcIrqRegiConf);
    /* Clear Pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_15);
    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
	
    /*start timer0*/
    TIMER0_Cmd(TMR_UNIT,Tim0_ChannelA,Enable);
	
	
	
}
void wifi_task(void *pvParameters)
{
	int time;
	static uint16_t clk=0;
	wifi_usart_init();
	time_init();

	
    while(WiFi_ConnectServer()){    //ѭ������ʼ�������ӷ�������ֱ���ɹ�
		vTaskDelay(2000);              //��ʱ
	}      
	WiFi_RxCounter=0;  

	//WiFi������������������                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);    //���WiFi���ջ�����               
	Connect_flag = 1;                          //Connect_flag=1,��ʾ�����Ϸ�����	
	for(;;)
	{
		if(Usart2_RxCompleted==1){		                          //���Usart2_RxCompleted����1����ʾ�����������
			Usart2_RxCompleted = 0;                               //�����־λ
			uint16_t ret= Data_buff[0]*256+Data_buff[1];
			uint8_t receive_asc_buff[200];
			hex_to_ascii(&Data_buff[2],receive_asc_buff,ret);
			u1_printf("����������%d�ֽ�����\r\n",(Data_buff[0]*256+Data_buff[1])); //���������Ϣ
			u1_printf("����:%s\r\n",receive_asc_buff);               //���������Ϣ   
			WiFi_printf("�����͵�������:%s\r\n",&Data_buff[2]);	  //�ѽ��յ������ݣ����ظ�������
			memset(receive_asc_buff,0,sizeof(receive_asc_buff));
	    }
		if(time>=1000){                                           //��time���ڵ���1000��ʱ�򣬴�ž���1s��ʱ��
			time=0;                                               //���time����
			WiFi_printf("���ǿͻ��ˣ��뷢������!\r\n");           //����������������
		}
		
		time++;       //time������+1
		clk++; 			
		/*��ӡ��Ϣ*/		
		if((clk%300)==0)
			debug_printf("wifi task runing:%d\n",clk/100);		
		vTaskDelay(3);
		//common_idle_judge(&wifi_usart2_frame);
		//data_analysis_handle(&wifi_usart2_frame,wifi_analysis_handle);
	}
}

/***********************************************END*****************************************************/

