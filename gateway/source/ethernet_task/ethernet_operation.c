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
#include "ethernet_data_handle.h"
#include "ethernet_operation.h"

#include "string.h"
#include "stdlib.h"
#include "debug_bsp.h"
#include "user_type.h"

///////////////////////////////////////////////����//////////////////////////////////////////
#include "wizchip_conf.h" //������Ҫ��ͷ�ļ�
#include "socket.h"       //������Ҫ��ͷ�ļ�
#include "dhcp.h"         //������Ҫ��ͷ�ļ�
#include "spi.h"          //������Ҫ��ͷ�ļ�
#include "dns.h"

#include "logic_control_task.h"
/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/

#define  IPPORT_DOMAIN	53

/*
*********************************************************************************************************
Typedef
*********************************************************************************************************
*/
typedef struct scoket_prot_handle_def
{
	u8 port;
	callback_u8_u8 handle;
	u8 close_flag;	
}scoket_prot_handle_t;
/*
*********************************************************************************************************
Variables
*********************************************************************************************************
*/
u16 ethernet_data_monitor_num = 0;
u8 	dns_server_ip[4] = {114,114,114,114};

u8 domain_name_buff[50];			
u8* domain_name = domain_name_buff;
u8 dns_finish_flag = 0;
extern system_information_t user_system_data;
char my_dhcp_retry = 0;         //DHCP��ǰ�������ԵĴ���
/*MAC��ַ����Ҫ��·�������������豸һ�� ���һλҪΪż��*/
wiz_NetInfo gWIZNETINFO =       
{        
	0x08, 0x08, 0xdc,0x00, 0xab, 0xc8, 
}; 
/*---------------------------------------------------------------*/
/*         ������IP��ַ�Ͷ˿ںţ������Լ�������޸�              */
/*---------------------------------------------------------------*/

/*Զ���޷���˿�*/
scoket_information_t user_scoket_cloud=
{	
	.dest_ip[0] = 106,
	.dest_ip[1] = 14,
	.dest_ip[2] = 201,
	.dest_ip[3] = 191,	
	.dest_prot = 5188,	
};
/*���ö˿� IP �Զ���ȡ*/
scoket_information_t user_scoket_set=
{
	.dest_ip[0] = 0,
	.dest_ip[1] = 0,
	.dest_ip[2] = 0,
	.dest_ip[3] = 208,
	.dest_prot = 5188,
};

/*  ����IP */
scoket_information_t user_scoket_balanced;


scoket_information_t user_scoket_prot_null=
{
	.dest_ip[0] = 192,
	.dest_ip[1] = 168,
	.dest_ip[2] = 1,
	.dest_ip[3] = 1,
	.dest_prot = 5188,
};
scoket_information_t *user_scoket_point_buff[8]=
{
	&user_scoket_prot_null,
	&user_scoket_cloud,	
	&user_scoket_prot_null,
	&user_scoket_set,
	&user_scoket_prot_null,
	&user_scoket_prot_null,
	&user_scoket_prot_null,
	&user_scoket_prot_null,
};

u8  dhcp_receive_buff[DATA_BUF_SIZE];    //���ݻ�����

static u8 scoket_connect_flag[8]={0,0,0,0,0,0,0,0}; 	/* 8��socket ����������*/	
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
u8 scoket_tcp_handle(u8 user_port ,callback_u8_u8 scoket_fuc);
u8 scoket_dns_handle(u8 user_port ,callback_u8_u8 scoket_fuc);
u8 scoket_cloud(u8 prot);
u8 scoket_set(u8 prot);
u8 scoket_udp(u8 prot);
u8 scoket_balanced(u8 prot);
scoket_prot_handle_t user_scoket_prot_handle[4]=
{
	{
		.port = SOCK_CLOUD,
		.handle = scoket_cloud,
		.close_flag = 0 ,
	},
	{
		.port = SOCK_LOCAL,
		.handle = scoket_set,
		.close_flag = 0 ,
	},
	{
		.port = SOCK_DNS,
		.handle = scoket_udp,
		.close_flag = 0 ,
	},
	{
		.port = SOCK_BALANCED,
		.handle = scoket_balanced,
		.close_flag = 0 ,
	}
	
};

void w5500_reset(void);
void w5500_set(void);
void cloud_data_monitor_reset(void);
void clear_connect_num(void);
/**
*********************************************************************************************************
* @����	: 
* @����	:  
*********************************************************************************************************
**/
void w5500_user_reset(void)
{
	w5500_reset();
	vTaskDelay(500);
	w5500_set();
	vTaskDelay(500);
}
/**
*********************************************************************************************************
* @����	: 
* @����	: ��ȡUID  д��MAC
*********************************************************************************************************
**/
void read_uid_num(void)
{
	u32 num;
	num = M4_EFM->UQID3;
	gWIZNETINFO.mac[0] = num>>24;
	gWIZNETINFO.mac[1] = num>>16;
	gWIZNETINFO.mac[2] = num>>8;
	gWIZNETINFO.mac[3] = num;	
	
	num = M4_EFM->UQID2;
	gWIZNETINFO.mac[4] = num>>8;
	gWIZNETINFO.mac[5] = ((num>>8)&0xf0)|0x04;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:  
*********************************************************************************************************
**/
void ethernet_driver_init(void)
{	
	/*��ȡUID д��MAC*/
	read_uid_num();
	/*��λW5500*/
	w5500_user_reset();	
	/*��ʼ��W5500*/
    w5500_init(); 	
	debug_printf("\r\n\r\n �������� \r\n\r\n"); /*��ʾ��Ϣ*/
	ethernet_task_state_channge(1);
	clear_connect_num();
}
/**
*********************************************************************************************************
* @����	: 
* @����	:  
*********************************************************************************************************
**/
void ethernet_run_reset(void)
{			
	/*��̫�������ʼ����λ����*/
	ethernet_task_state_channge( 0 );	
}
/**
*********************************************************************************************************
* @����	: 
* @����	:  
*********************************************************************************************************
**/
u8 iap_time_handle(u8 cmd);
void ethernet_run(void)
{
	static u16 connect_count = 0;	
	 /*DHCPִ��*/
	switch(DHCP_run())          
	{
		case DHCP_IP_ASSIGN:     /*·��������ip*/
		case DHCP_IP_CHANGED:    /*·�����ı��˷���ip*/
		{
			connect_count = 0;
			my_ip_assign();   	/*����IP������ȡ����*/
		}
		break;           
		/*·��������ip����ʽ�����ˣ���������ͨ��*/
		case DHCP_IP_LEASED:  
		{
			connect_count = 0;	
			/*ʹ���ƶ��ϱ�*/
			if(user_system_data.set.cloud_selection == 1)			
				scoket_tcp_handle(user_scoket_prot_handle[0].port , user_scoket_prot_handle[0].handle); /*�����ƶ˷�����SCOKET ����*/
			/*�˿��Ƿ�ر�*/
			if(user_scoket_prot_handle[1].close_flag == 0)
				scoket_tcp_handle(user_scoket_prot_handle[1].port , user_scoket_prot_handle[1].handle); /*�������÷�����SCOKET ����*/
			/*�˿��Ƿ�ر�*/
			if(user_scoket_prot_handle[2].close_flag == 0)
				scoket_dns_handle(user_scoket_prot_handle[2].port , user_scoket_prot_handle[2].handle);/*����DNS��������SCOKET ����*/
			/*�˿��Ƿ�ر�*/
			if((user_scoket_prot_handle[3].close_flag == 0) && (user_system_data.set.cloud_selection == 1))
				scoket_tcp_handle(user_scoket_prot_handle[3].port , user_scoket_prot_handle[3].handle);/*������⸺��IP��ȡSCOKET ����*/			
		}
		break;
		/*��ȡIPʧ�� */
		case DHCP_FAILED:   
		{	
			connect_count = 0;			
			my_dhcp_retry++;                   //ʧ�ܴ���+1
			if(my_dhcp_retry > MY_MAX_DHCP_RETRY)
			{  
				
				debug_printf("DHCPʧ�ܣ�׼������\r\n");
				ethernet_run_reset(); /* W5500��ʼ����λ���� */		    
				debug_printf("�������IPʧ��\r\n");
			}
		}
		break;		
	}
	connect_count++;
	if(connect_count > 3000)
	{
		connect_count = 0;
		ethernet_run_reset();
		debug_printf("*����DHCP�����쳣����λW5500!\r\n"); /*��ʾ��Ϣ*/		
	}
	/*��ʱ����*/
	cloud_data_monitor_reset();	
	/*��ʱ����*/
	vTaskDelay(1);
	iap_time_handle(0);
}
/**
*********************************************************************************************************
* @����	: 
* @����	:1����û���յ��ƶ����� ��λ
*********************************************************************************************************
**/
void cloud_data_monitor_reset(void)
{
	ethernet_data_monitor_num++;
	/*1����û���յ��ƶ����� ��λ*/
	if((ethernet_data_monitor_num > 60000)&&(user_system_data.set.cloud_selection == 1))
	{
		/*��̫����������*/
		ethernet_task_state_channge( 0 );
		/*����״̬*/
		ethernet_data_monitor_num = 0 ;
		memset(scoket_connect_flag, 0 ,sizeof(scoket_connect_flag));
		debug_printf("������������\r\n");
		debug_printf("��������ƶ�������\r\n");
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ռ�����
*********************************************************************************************************
**/
void cloud_data_monitor_clear(void)
{
	ethernet_data_monitor_num = 0; 
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ռ�����
*********************************************************************************************************
**/
u8  get_cloud_connect_state(void)
{
	if(scoket_connect_flag[SOCK_CLOUD] == 1)
		return 1;
	return 0;
}
/**
*********************************************************************************************************
* @����	: 
* @����	:�رվ��⸺��
*********************************************************************************************************
**/
u8 scoket_balanced_flag = 0;
void close_balanced(u8 cmd)
{
	user_scoket_prot_handle[3].close_flag = 1;
	scoket_balanced_flag = cmd; 
	close(SOCK_BALANCED);
}
/**
*********************************************************************************************************
* @����	: 
* @����	:��ռ�����
* @����	: 
* @����	: 
*********************************************************************************************************
**/
#define CONNECT_NUM 8
static u8 connect_num[CONNECT_NUM] ;
static u8 lcoal_ip_num=0;
void clear_connect_num(void)
{
	lcoal_ip_num = 0;
	for(u8 i = 0 ; i < CONNECT_NUM ; i++)
	{
		connect_num[i] = 0;
	}
}
/**
*********************************************************************************************************
* @����	: 
* @����	: SCOKET ���Ӵ���   
����TCP��̵Ŀͻ���һ�㲽���ǣ� 
����1������һ��socket���ú���socket()�� 
����2������Ҫ���ӵĶԷ���IP��ַ�Ͷ˿ڣ����ӷ��������ú���connect()��
����3���շ����ݣ��ú���send()��recv()��
����4���ر��������ӣ�
*********************************************************************************************************
**/
#define SOCKET_DELAY (1000)

u8 scoket_tcp_handle(u8 user_port ,callback_u8_u8 scoket_fuc)
{
	static u16 scoket_delay_buff[8]={0,0,0,0,0,0,0,0}; /* 8��socket ��ʱ*/
	static u16 scoket_restart_buff[8]={0,0,0,0,0,0,0,0}; /* 8��socket ����������*/	

	int ret;
	static u16 dns_delay=0;
	if(user_port > 7) /*�˿ڱ���С�ڵ���7*/
		return 0;
	if(scoket_delay_buff[user_port] == 0)
	{
		ret=getSn_SR(user_port);       /*��ȡTCP����״̬*/    
		switch(ret)     
		{
			/*���ڹرս׶Σ�����SOCKET*/					
			case SOCK_CLOSED:          
			{
				 debug_printf("SOCK%d׼���򿪱��ض˿�\r\n",user_port);                
				 ret = socket(user_port, Sn_MR_TCP, 5188, Sn_MR_ND); /*����һ��SCOKET TCP����*/
				/*����ֵ�����ڵ�ǰʹ�ö˿ڣ���ʾ�򿪴���*/
				 if(ret != user_port)                             
				 {
					debug_printf("SOCK%d�˿ڴ���׼������\r\n",user_port);         

					scoket_delay_buff[user_port]  = SOCKET_DELAY;
					scoket_restart_buff[user_port]++;
					 /* 10��ɨ��� W5500��ʼ������ */
					if(scoket_restart_buff[user_port] > 3)
					{
						ethernet_run_reset();
						debug_printf("������϶˿ڴ���\r\n");
					}						
				 }
				 else
					 scoket_restart_buff[user_port] = 0;
				 scoket_connect_flag[user_port] = 0 ;
				 debug_printf("SOCK%d�򿪱��ض˿ڳɹ�\r\n",user_port);                
			}
			break;
			 /*SOCKET��ʼ���ɹ���׼������*/ 
			case SOCK_INIT:         
			{					
				if(user_port == SOCK_CLOUD )
				{
					 /*�ȴ� DNS��������*/ 
					if(scoket_balanced_flag != 0) 			
					{						
						/*���Ӿ��⸺��*/
						if(scoket_balanced_flag == 1)
						{
							debug_printf("SOCK%d��������IP\r\n", user_port); 					
							ret = connect(user_port, user_scoket_balanced.dest_ip , user_scoket_balanced.dest_prot); 
						}
						/*����IP*/
						else		
						{
							debug_printf("SOCK%d��������IP\r\n", user_port); 
							ret = connect(user_port, user_system_data.set.final_server_ip, user_system_data.set.final_serve_port[0]*256+user_system_data.set.final_serve_port[1]); 
						}
						connect_num[user_port]++;
						if(ret == SOCKERR_NOPEN)/*�жϷ�����δ����*/
						{
							debug_printf("SOCK%d������δ��������%d������ʧ�ܣ�10s��׼����������,\r\n", user_port , connect_num[user_port]);  
							scoket_delay_buff[user_port]  = SOCKET_DELAY*3;
							/*�������ʧ�ܣ�������  �ƶ�10������ʧ������  ���ض�5������ʧ�ܸ���IP 28 68 88 */					
							if(connect_num[user_port] > 10)
							{
								debug_printf("SOCK%d�����ƶ˷�����ʧ�ܣ��豸��������\r\n",user_port); 
								ethernet_run_reset();
								debug_printf("��������ƶ˷���������\r\n");
							}								
						}
					}										
				}
				else if(user_port == SOCK_LOCAL )
				{
					debug_printf("SOCK%d׼�����ӷ�����\r\n",user_port); 
					ret = connect(user_port, user_scoket_point_buff[user_port]->dest_ip, user_scoket_point_buff[user_port]->dest_prot); 

					connect_num[user_port]++;
					if(ret == SOCKERR_NOPEN)/*�жϷ�����δ����*/
					{
						debug_printf("SOCK%d������δ��������%d������ʧ�ܣ�10s��׼����������,\r\n", user_port , connect_num[user_port]);  
						scoket_delay_buff[user_port]  = SOCKET_DELAY*3;
					}						
				}
				if(user_port == SOCK_BALANCED )
				{
					 /*�ȴ� DNS��������*/ 
					if(dns_finish_flag == 0) 
					{
						if((dns_delay++%500) == 0)
						{
							debug_printf("��%d�εȴ�DNS��������\r\n",dns_delay/300); 
						}
						if(dns_delay >= 3000)
						{
							debug_printf("DNS��������ʧ��\r\n",dns_delay/300); 
							dns_finish_flag = 2;
						}
						
					}
					else			
					{							
						/*������������IP*/
						if(dns_finish_flag == 1)
						{
							debug_printf("SOCK%d��������IP\r\n", user_port); 					
							ret = connect(user_port, DNS_GET_IP , user_system_data.set.load_balancing_port[0]*256 + user_system_data.set.load_balancing_port[1]); 
							connect_num[user_port]++;
							if(ret == SOCKERR_NOPEN)/*�жϷ�����δ����*/
							{
								debug_printf("SOCK%d������δ��������%d������ʧ�ܣ�10s��׼����������,\r\n", user_port , connect_num[user_port]);  
								scoket_delay_buff[user_port]  = SOCKET_DELAY*3;
								/*�������ʧ�ܣ�������  0������ʧ������ ���ñ�־λ �ر�socket  */					
								if(connect_num[user_port] > 10)
								{
									close_balanced(2);									
								}								
							}
						}
					}										
				}						
			}
			break;			
			/*���ӽ����ɹ�  ���� ��������*/ 
			case SOCK_ESTABLISHED:   
			{
				/*�ж�һ�����ӳɹ����*/		
				if((scoket_connect_flag[user_port] == 0) && (getSn_IR(user_port) == Sn_IR_CON))   
				{
					scoket_connect_flag[user_port] = 1;
					debug_printf("SOCK%d�����ѽ���\r\n",user_port);      /*������ʾ��Ϣ*/
					connect_num[user_port] = 0;
				}
				 /*���� �������ݻص�����*/	
				(*scoket_fuc)(user_port);		
			}
			break; 
			
			/*�ȴ��ر�����*/							 
			case SOCK_CLOSE_WAIT:      
			{
				debug_printf("SOCK%d�ȴ��ر�����\r\n",user_port);   
				if(( ret = disconnect(user_port) ) != SOCK_OK)  /*�ر����ӣ����жϹرճɹ����*/
				{
					debug_printf("SOCK%d���ӹر�ʧ�ܣ�׼������\r\n",user_port);       /*��ʾ��Ϣ*/

					scoket_delay_buff[user_port]  = SOCKET_DELAY;
					scoket_restart_buff[user_port]++;
					if(scoket_restart_buff[user_port] > 3)
					{
						ethernet_run_reset();	                     /*����*/
						debug_printf("�����������ʧ��\r\n");
					}
				 }
				 else
					 scoket_restart_buff[user_port] = 0;
				scoket_connect_flag[user_port] = 0; 
				debug_printf("SOCK%d���ӹرճɹ�\r\n",user_port);         /*��ʾ�رճɹ�*/
			}
			break;
		
			default: 						
			break;      
		}
	}
	else
	{
		if(scoket_delay_buff[user_port]  > 0)
			scoket_delay_buff[user_port]--;	
		vTaskDelay(1);
	}
	return 0;
}
/**
*********************************************************************************************************
* @����	: 
* @����	: SCOKET ���Ӵ���   UDP  
*********************************************************************************************************
**/
u8 scoket_dns_handle(u8 user_port ,callback_u8_u8 scoket_fuc)
{	
	static u16 scoket_delay_buff[8]={0,0,0,0,0,0,0,0}; /* 8��socket ��ʱ*/
	static u16 dns_wait_time = 0;
	int  ret;                       //���ڱ��溯������ֵ
	u16 len, port;
	struct dhdr dhp;
	static u8 udp_dns_buff[200];
	
	/*�˿ڱ���С�ڵ���7*/
	if(user_port > 7) 
		return 0;
	if((scoket_delay_buff[user_port] == 0) && (dns_finish_flag == 0))
	{
		/*��ȡ����״̬*/ 
		ret=getSn_SR(user_port);          
		switch(ret)     
		{
			/*SOCK��ʼ���ɹ������Խ�������*/   
			case SOCK_UDP:           
				
				if ((len = getSn_RX_RSR(user_port)) > 0)
				{
					if (len > MAX_DNS_BUF_SIZE) 
						len = MAX_DNS_BUF_SIZE;
					len = recvfrom(user_port, udp_dns_buff, len, dns_server_ip, &port);
					
					debug_printf("�յ�DNS����%d\r\n",len);      /*������ʾ��Ϣ*/
					if(parseMSG(&dhp, udp_dns_buff))
					{
						debug_printf("���IP������%d.%d.%d.%d\r\n",DNS_GET_IP[0],DNS_GET_IP[1],DNS_GET_IP[2],DNS_GET_IP[3]);
						close(user_port);
						user_scoket_prot_handle[2].close_flag = 1;
						debug_printf("�ر�SOCK\r\n");
						dns_finish_flag = 1;
					}
					else
					{
						debug_printf("������������\r\n");
					}
				}
				else
				{
					#define DNS_TIME_NUM 300
					if(( dns_wait_time++%DNS_TIME_NUM ) == 0)
					{
						/*��DNS����������*/
						len = dns_makequery(0, domain_name, udp_dns_buff, MAX_DNS_BUF_SIZE);
						debug_printf("��%d����DNS��������%s\r\n", dns_wait_time/DNS_TIME_NUM, domain_name);    
						sendto(user_port, udp_dns_buff, len, dns_server_ip, IPPORT_DOMAIN);									
					}
					if(dns_wait_time>1000)
					{
						dns_finish_flag = 2;
						close(user_port);
						debug_printf("DNS��������Ӧ�𣬹رն˿ڣ�����������%s\r\n");  							
					}				
				}					
			break;
				
			/*SOCK���ڹر�״̬�����Դ�*/ 
			case SOCK_CLOSED:        			
				debug_printf("SOCK%d׼���򿪶˿�\r\n",user_port);          
				/*��socket�˿� UDP ģʽ*/ 
				ret = socket(user_port,Sn_MR_UDP,5188,0);
				/*��ʾ�򿪴���*/ 
				if(ret != user_port)                             
				{
					debug_printf("SOCK%d�˿ڴ���׼������\r\n",user_port);         
					scoket_delay_buff[user_port] = SOCKET_DELAY;                             
					ethernet_run_reset();	 
					debug_printf("������϶˿ڹر�\r\n");                    
				}
				debug_printf("SOCK%d�򿪱��ض˿ڳɹ�\r\n",user_port); 
				
				/*��DNS����������*/
				memcpy(domain_name, user_system_data.set.domain_name, user_system_data.set.domain_name_length);
				len = dns_makequery(0, domain_name, udp_dns_buff, MAX_DNS_BUF_SIZE);
				debug_printf("��DNS��������%s\r\n",domain_name);    
				sendto(user_port, udp_dns_buff, len, dns_server_ip, IPPORT_DOMAIN);				
				
			break;                                         

			default:               
			break;         
		}
	}
	else
	{
		if(scoket_delay_buff[user_port]  > 0)
			scoket_delay_buff[user_port]--;	
		vTaskDelay(1);
	}
	return 0;
}
/**
*********************************************************************************************************
* @����	: 
* @����	: ��ȡ��IPʱ�Ļص����� 
*********************************************************************************************************
**/
void my_ip_assign(void)
{
   getIPfromDHCP(gWIZNETINFO.ip);     //�ѻ�ȡ����ip��������¼����������
   getGWfromDHCP(gWIZNETINFO.gw);     //�ѻ�ȡ�������ز�������¼����������
   getSNfromDHCP(gWIZNETINFO.sn);     //�ѻ�ȡ�������������������¼����������
   getDNSfromDHCP(gWIZNETINFO.dns);   //�ѻ�ȡ����DNS��������������¼����������
   gWIZNETINFO.dhcp = NETINFO_DHCP;   //���ʹ�õ���DHCP��ʽ
   network_init();                    //��ʼ������  
   debug_printf("DHCP���� : %d ��\r\n", getDHCPLeasetime());
}
/**
*********************************************************************************************************
* @����	: 
* @����	: ��ȡIP��ʧ�ܺ��� 
*********************************************************************************************************
**/
void my_ip_conflict(void)
{
	debug_printf("��ȡIPʧ�ܣ�׼������\r\n");   //��ʾ��ȡIPʧ��
	w5500_user_reset();                      //����
}
/**
*********************************************************************************************************
* @����	: 
* @����	: ��ʼ�����纯��  
*********************************************************************************************************
**/
void network_init(void)
{
	char tmpstr[6] = {0};
	wiz_NetInfo netinfo;
	
	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);//�����������
	ctlnetwork(CN_GET_NETINFO, (void*)&netinfo);	//��ȡ�������
	ctlwizchip(CW_GET_ID,(void*)tmpstr);	        //��ȡоƬID

	//��ӡ�������
	if(netinfo.dhcp == NETINFO_DHCP) 
		debug_printf("\r\n=== %s NET CONF : DHCP ===\r\n",(char*)tmpstr);
	else 
		debug_printf("\r\n=== %s NET CONF : Static ===\r\n",(char*)tmpstr);	
    debug_printf("===========================\r\n");
	debug_printf("MAC��ַ: %02X:%02X:%02X:%02X:%02X:%02X\r\n",netinfo.mac[0],netinfo.mac[1],netinfo.mac[2],netinfo.mac[3],netinfo.mac[4],netinfo.mac[5]);			
	debug_printf("IP��ַ: %d.%d.%d.%d\r\n", netinfo.ip[0],netinfo.ip[1],netinfo.ip[2],netinfo.ip[3]);
	debug_printf("���ص�ַ: %d.%d.%d.%d\r\n", netinfo.gw[0],netinfo.gw[1],netinfo.gw[2],netinfo.gw[3]);
	debug_printf("��������: %d.%d.%d.%d\r\n", netinfo.sn[0],netinfo.sn[1],netinfo.sn[2],netinfo.sn[3]);
	debug_printf("DNS������: %d.%d.%d.%d\r\n", netinfo.dns[0],netinfo.dns[1],netinfo.dns[2],netinfo.dns[3]);
	debug_printf("===========================\r\n");
	
	user_scoket_set.dest_ip[0] = netinfo.ip[0];
	user_scoket_set.dest_ip[1] = netinfo.ip[1];
	user_scoket_set.dest_ip[2] = netinfo.ip[2];

}
/**
*********************************************************************************************************
* @����	: 
* @����	: ��ʼ��W5500  
*********************************************************************************************************
**/
void w5500_init(void)
{
	//W5500�շ��ڴ�������շ������������ܵĿռ���16K����0-7��ÿ���˿ڵ��շ����������Ƿ���
    char memsize[2][8] = {{2,2,2,4,2,2,1,1},{2,2,2,4,2,2,1,1}}; 
	char tmp;
	u16 clk = 0;
	SPI_Configuration();                                    //��ʼ��SPI�ӿ�
	reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);	//ע���ٽ�������
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);  //ע��SPIƬѡ�źź���
	reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);	//ע���д����
	if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1)
	{   //���if��������ʾ�շ��ڴ����ʧ��
		 debug_printf("��ʼ���շ�����ʧ��,׼������\r\n");      //��ʾ��Ϣ
  		 w5500_user_reset();                                //����
	}
	/*�������״̬*/
    do
	{                                                    
		if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1)/*�������״̬*/
		{ 		 
			debug_printf("δ֪����׼������\r\n");            
			w5500_user_reset();                             //����
		}
		if(tmp == PHY_LINK_OFF)
		{
			if((clk%20) == 0)
			{
				debug_printf("����δ����\r\n");//�����⵽������û���ӣ���ʾ��������
				vTaskDelay(100);              //��ʱ
			}
			/*60������ ��λ����*/
			if(clk > 600)
				break;
		}
		vTaskDelay(100);
		clk++;
	}while(tmp == PHY_LINK_OFF);                            

	/*����MAC��ַ*/
	setSHAR(gWIZNETINFO.mac);                                   //����MAC��ַ
	DHCP_init(SOCK_DHCP, dhcp_receive_buff);                     //��ʼ��DHCP
	reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);//ע��DHCP�ص����� 
    my_dhcp_retry = 0;	                                        //DHCP���Դ���=0
}


///***********************************************END*****************************************************/

