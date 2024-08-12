#include "usart_1.h"
#include <string.h>

USART1_TypeDef usart1;

/*********************************************************************************************************
* 函 数 名 : Usart1_SendString
* 功能说明 : USART1发送字符串函数
* 形    参 : str：需要发送的字符串
* 返 回 值 : 无
* 备    注 : 无
*********************************************************************************************************/ 
void Usart1_SendString(unsigned char *str)
{
	while(*str != 0)		
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	//等待上一个字节发送完成
		USART_SendData(USART1, *str++);								//发送一个字节
	}
}
/*********************************************************************************************************
* 函 数 名 : Usart1_SendPackage
* 功能说明 : USART1数据包发送函数
* 形    参 : data：需要发送的数据，len：发送的数据的长度
* 返 回 值 : 无
* 备    注 : 数据包中间可能会包含‘\0’, 所以需要依赖长度进行发送
*********************************************************************************************************/ 
void Usart1_SendPackage(unsigned char *data, unsigned short len)	
{
	while(len--)
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	//等待上一个字节发送完成
		USART_SendData(USART1, *data++);							//发送一个字节
	}
}

/***************************************************************************************************
 函数名称：void USART_OutPut(USART_TypeDef* USARTx,u8 c)
 功能描述：单字节输出
 参数描述：
*****************************************************************************************************/
void USART_OutPut(USART_TypeDef* USARTx, u8 c)
{
	USARTx->DR = (c & 0x01FF);
// 	while((USARTx->SR&USART_FLAG_TXE)==RESET);  //数据还没有被转移到移位寄存器为0，数据被转移到移位寄存器为1
}

/*********************************************************************************************************
* 函 数 名 : USART1_IRQHandler
* 功能说明 : USART1中断服务函数
* 形    参 : 无
* 返 回 值 : 无
* 备    注 : 接收中断(+空闲中断), 每接收到一个字节，就会执行一次
* 格式：ZXI_ ...200bytes 0D 0A
*********************************************************************************************************/ 
u16 USART_RX_STA = 0;       //接收状态标记

#define GPRSReceCount_LEN	20
static u8 __IO GPRSReceCount1 = 0;
static u8  GPRS_Response1[20];

u16 GPRSReceCount = 0;
char GPRS_Response[1500];			//接收数组
u8 GPRSReceResultCount = 0;
char GPRSCheck_Result[100];

#define GPRS_GET_OK		0x2E
#define CMD_QIREAD		0x35
u8 GPRSResponseState = NULL;	//GPRS响应状态
u8 GPRSCmdState = NULL;

uint8_t start_flag = 0;

uint8_t 	Rx1_Buff[Rev1_Buff_len];	//串口1接收缓冲数组
uint8_t 	Tx1_Buff[Sen1_Buff_len];	//串口1发送缓冲数组
uint32_t  Rx1_count = 0;						//串口1接收指针
uint32_t  Tx1_count = 0;						//串口1发送指针

uint8_t Refinish1_Flag = 0;     //串口1接收字符串完成标志位（遇到约定字符即表示接收完成）
uint8_t error_type = 0;       //错误类型
uint8_t ins[] = "ZXI_"; //"@PrintData"; //指令头“ZXI_HEAD/ZXI_DATA”，指令尾“0#”
uint8_t len = sizeof(ins)/sizeof(uint8_t)-1; //指令长度

uint8_t error1[] = "Overflow Error\r\n";
uint8_t error2[] = "Instruction Error\r\n";
uint8_t error3[] = "校验错误，请重新发送\r\n";
uint8_t sucess[] = "数据已成功接收\r\n";

/*以中断方式接收数据*/
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE) != Bit_RESET)//检查指定的USART接收中断发生与否（接收数据寄存器数据不为空）
	{
		USART1_CharReception_Callback();  //接收中断回调函数
	}
	if(USART_GetITStatus(USART1,USART_IT_PE) != Bit_RESET)
	{
		USART1_ParityError_Callback();    //校验错误中断回调函数
	}
	if(USART_GetITStatus(USART1,USART_IT_IDLE) != Bit_RESET)//检查指定的USART空闲中断发生与否（常用于判断数据接收结束）
	{
		USART1_ReceptionIDLE_Callback();  //空闲中断回调函数
	}
}

/*********************************************************************************************************
* 函 数 名 : Usart1_Init
* 功能说明 : 初始化USART1
* 形    参 : bound：波特率
* 返 回 值 : 无
* 备    注 : 无
* 2024-7-10,添加串口的中断配置
*********************************************************************************************************/ 
void Usart1_Init(unsigned int nBaudRate)
{
	  //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
	 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);	//使能USART1，GPIOA时钟以及复用功能时钟
     //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  

   //Usart1 NVIC 配置
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);  //抢占优先级1位（0~1），响应优先级3位（0~7）
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

		USART_InitStructure.USART_BaudRate = nBaudRate;//一般设置为9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
		USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
		USART_Init(USART1, &USART_InitStructure); //初始化串口
		
		USART_Cmd(USART1, ENABLE);		   // 使能USART1
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//使能或者失能指定的USART中断 接收中断
		USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//使能或者失能指定的USART中断 空闲中断
		USART_ITConfig(USART1, USART_IT_PE, ENABLE);	//使能或者失能指定的USART中断 校验错误中断
		//如果使用的串口发送的功能，一定加上下面这句话，防止发送数据时被单片硬件缓冲区已存在的数据混合，导致出错
		USART_ClearFlag(USART1,USART_FLAG_TC);   //清除发送完成标志位
}

/*******************************************************************************
* 函 数 名         : clearData
* 函数功能		     : 清空缓冲区数据
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/	
void clear_BuffData(void)
{
	memset(Rx1_Buff, '\0', sizeof(Rx1_Buff)); //清空串口1接收缓冲区
	memset(Tx1_Buff, '\0', sizeof(Tx1_Buff)); //清空串口1发送缓冲区
	Rx1_count = 0;  
	Tx1_count = 0;
	error_type = 0;
}

/*******************************************************************************
* 函 数 名         : Send_data
* 函数功能		     : 串口发送多字节数据
* 输    入         : uint8_t *data 待发送数据
* 输    出         : 无
*******************************************************************************/	
void Send_data(uint8_t *data)
{
	while(*data!='\0')
	{ 
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC )==RESET);	
		USART_SendData(USART1,*data);
		data++;
	}
}


/*******************************************************************************
* 函 数 名         : Parse_command
* 函数功能		     : 串口数据解析
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void Parse_data(void)
{
	if(Rx1_count >= Rev1_Buff_len - 1)   //达到缓冲区最大接收长度    
	{
		error_type = 1;  //溢出错误
	}
	else
	{
		if((Rx1_Buff[Rx1_count-1] == '#')&&(Rx1_Buff[Rx1_count-2] == '0')) //判断结束位
		{
			static uint32_t i;
			static uint32_t j;
			i = 0;
			while(i<len)
			{
				if(Rx1_Buff[i]== ins[i])
				{
					i++;
				}
				else  //，没有以@PrintData开头，指令错误
				{
					error_type = 2;
					break;
				}
			}
			if(i==len) //指令正确
			{
				for(j=len;j<Rx1_count-2;j++)
				{
					Tx1_Buff[j-len] = Rx1_Buff[j];  //取出中间的数据部分
				}	
				//Tx1_Buff[Rx1_count-2-len] = '\r'; //2024-8-4, comment
				//Tx1_Buff[Rx1_count-1-len] = '\n'; //换行2024-8-4, comment
			}
		}
		else   //没有以“0#”结束，指令错误
		{
			error_type = 2; //指令错误
		}
	}
}

/*******************************************************************************
* 函 数 名         : UsartdisData
* 函数功能		     : 串口显示数据
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/	
void UsartdisData(void)
{
	if(error_type == 0)        //无错误
	{
		//Send_data(Tx1_Buff);
		printf("ZXI>Rx: [0x%08X]%c%c%c%c, No.:%03d, PkgNo:%d\r\n", (Tx1_Buff[5] - 1) * 200 + 8, Tx1_Buff[0], Tx1_Buff[1], Tx1_Buff[2], Tx1_Buff[3], Tx1_Buff[4], Tx1_Buff[5]);
		usart1.RxBuff[0] = 'Z';
		usart1.RxBuff[1] = 'X';
		usart1.RxBuff[2] = 'I';
		usart1.RxBuff[3] = '_';
		for (int i = 0; i < USART1_RX_BUFF_MAX - 4; i++) {
			usart1.RxBuff[4 + i] = Tx1_Buff[i];
		}
		for(int i = 0; i < 16; i++) {
			printf("%02X ", Tx1_Buff[6 + i]);
		}
		Send_data(sucess);
	}
	else if(error_type == 2) //指令错误
	{
		Send_data(error2);
	}
	else if(error_type == 1) //溢出错误
	{
		Send_data(error1);
	}
	else if(error_type == 3)
	{
		Send_data(error3);  //校验错误
	}
	clear_BuffData(); //清除缓冲数据
}


/*******************************************************************************
* 函 数 名         : USART1_CharReception_Callback
* 函数功能		     : 串口1接收数据触发函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/	
void USART1_CharReception_Callback(void)
{
	Rx1_Buff[Rx1_count++] = USART_ReceiveData(USART1);	   //接收数据到缓冲区		
	if(error_type == 3) //清除校验错误标志位
	{
		USART1->SR;  //先读状态寄存器
		USART1->DR;  //再读数据寄存器
	}		
}

/*******************************************************************************
* 函 数 名         : USART1_ReceptionIDLE_Callback
* 函数功能		     : 串口1总线空闲触发函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/	
void USART1_ReceptionIDLE_Callback(void)
{
	//两条语句清除IDLE中断(读走数据后，清除空闲中断标志位)
	USART1->SR;  //先读状态寄存器
	USART1->DR;  //再读数据寄存器
	Refinish1_Flag = 1;  //接收结束标志位
}

/*******************************************************************************
* 函 数 名         : USART1_ParityError_Callback
* 函数功能		     : 校验出错回调函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/	
void USART1_ParityError_Callback(void)
{
	error_type = 3;  //校验错误
}
/**************************************************************************************************************/

/*****加入以下代码,支持printf函数,而不需要选择use MicroLIB*****/	  
#if 1
	#pragma import(__use_no_semihosting)             
	//标准库需要的支持函数                 
	struct __FILE 
	{ 
		int handle; 
	}; 

  FILE __stdout;       
	//定义_sys_exit()以避免使用半主机模式    
	void _sys_exit(int x) 
	{ 
		x = x; 
	} 
	void _ttywrch(int ch)
	{
		ch = ch;
	}
	//重定义fputc函数 
	int fputc(int ch, FILE *f)
	{ 	
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
		USART_SendData(USART1, ch);	
		return ch;
	}
/**********************printf support end**********************/	  
#endif
