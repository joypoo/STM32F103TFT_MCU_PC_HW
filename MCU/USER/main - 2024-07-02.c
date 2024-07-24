//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ABrobot智能
//
//
//  文 件 名   : main.c
//  版 本 号   : v2.0
//  作    者   : ABrobot
//  生成日期   : 2023-10-25
//  最近修改   : 
//  功能描述   :演示例程(STM32F103 TFT开发板系列 京东方玻璃)
//              
//							屏幕接线
//              ----------------------------------------------------------------
//              GND   电源地
//              VCC   3.3v电源
//              SCL   PB3（SCLK）
//              SDA   PB5（MOSI）
//              RES   PB6
//              DC    PB4
//              CS    PB7
//              
//              ----------------------------------------------------------------
//
//							FLASH芯片接线
//              ----------------------------------------------------------------
//
//              GND   电源地
//              VCC   3.3v电源
//              CS    PB12
//              CLK   PB13
//              DO    PB14
//              DI    PB15
//             
//              
//              ----------------------------------------------------------------
//版权所有，盗版必究。
//All rights reserved
//******************************************************************************/
#include "delay.h"
#include "sys.h"
#include "led.h"
#include "lcd_init.h"
#include "lcd.h"
#include "pic.h"
#include "usart_1.h"
#include "W25QXX.h"

#define SingleKeyEvent

//单个按键检测
#ifdef SingleKeyEvent

//按键对应的IO管脚 KEY1  PA.15
#define KEY_IO_RCC	RCC_APB2Periph_GPIOA      
#define KEY_IO_PORT	GPIOA
#define KEY_IO_PIN	GPIO_Pin_0

#define Key	GPIO_ReadInputDataBit(KEY_IO_PORT, KEY_IO_PIN)

typedef enum
{
    KEY_CHECK = 0,
    KEY_COMFIRM = 1,
    KEY_RELEASE = 2 
}KEY_STATE;

KEY_STATE KeyState = KEY_CHECK;	// 初始化按键状态为检测状态
u8 g_KeyFlag = 1;								// 按键有效标志，0： 按键值无效； 1：按键值有效

/**
 * 单个按键检测事件
 * 功能：使用状态机方式，扫描单个按键；扫描周期为10ms,10ms刚好跳过抖动；
 * 状态机使用switch case语句实现状态之间的跳转
 * 
 */
void Key_Scan(void)
{
		switch (KeyState)
    {
        //按键未按下状态，此时判断Key的值
        case	KEY_CHECK:    
            if(!Key)   
            {
                KeyState =  KEY_COMFIRM;	//如果按键Key值为0，说明按键开始按下，进入下一个状态
            }
            break;
				//按键按下状态：
        case	KEY_COMFIRM:
            if(!Key)											//查看当前Key是否还是0，再次确认是否按下
            {
                KeyState =  KEY_RELEASE;	//进入下一个释放状态
                g_KeyFlag = 1;						//按键有效值为1, 按键确认按下，按下就执行按键任务；        
            }   
            else													//当前Key值为1，确认为抖动，则返回上一个状态
            {
                KeyState =  KEY_CHECK;		//返回上一个状态
            } 
            break;
				//按键释放状态
        case	KEY_RELEASE:
             if(Key)                     //当前Key值为1，说明按键已经释放，返回开始状态
             { 
                 KeyState =  KEY_CHECK;
               //  g_KeyFlag = 1;        //如果置于此，则在按键释放状态后，才执行按键任务；
							 g_KeyFlag = 0;
             } 
             break;
         default: 
					 break;
    }
}

#endif

//-----------------

typedef enum { FAILED = 0, PASSED = !FAILED } TestStatus;

/* 获取缓冲区的长度 */
#define TxBufferSize1   (countof(TxBuffer1) - 1)
#define RxBufferSize1   (countof(TxBuffer1) - 1)
#define countof(a)      (sizeof(a) / sizeof(*(a)))
#define	BufferSize		(countof(Tx_Buffer)-1)

#define  FLASH_WriteAddress     0x00000
#define  FLASH_ReadAddress      FLASH_WriteAddress
#define  FLASH_SectorToErase    FLASH_WriteAddress

#define Erase 0
uint8_t Tx_Buffer[] = "123456";


uint8_t Rx_Buffer[BufferSize];
__IO uint32_t DeviceID = 0;
__IO uint32_t FlashID = 0;
__IO TestStatus TransferStatus1 = FAILED;

// 函数原型声明
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);

int main(void)
{
	float t = 0;
	__IO uint32_t Flash_Size = 0;
	delay_init();
	LED_Init();//LED初始化
	Usart1_Init(115200);
	SPI_FLASH_Init();

	LCD_Init();//LCD初始化
	//LCD_BLK_Set();//打开背光
	LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
	
	LED = 1;


	/* 获取 Flash Device ID */
	DeviceID = SPI_FLASH_ReadDeviceID();

	/* 获取 SPI Flash ID */
	FlashID = SPI_FLASH_ReadID();
	printf("\r\n FlashID is 0x%X \r\n Device ID is 0x%X\r\n", FlashID, DeviceID);


	/* 检验 SPI Flash ID */
	if (FlashID == W25Q16ID || W25Q32ID || W25Q64ID || W25Q80ID)
	{

		printf("\r\n 检测到串行flash芯片!\r\n");
		switch (FlashID)
		{
		case W25Q16ID:
			printf("\r\n flash芯片型号为W25Q16ID!\r\n");
			Flash_Size = 2;
			break;
		case W25Q32ID:
			printf("\r\n flash芯片型号为W25Q32!\r\n");
			Flash_Size = 4;
			break;
		case W25Q64ID:
			printf("\r\n flash芯片型号为W25Q64!\r\n");
			Flash_Size = 8;
			break;
		case W25Q80ID:
			printf("\r\n flash芯片型号为W25Q80!\r\n");
			Flash_Size = 1;
			break;
		default:
			printf("\r\n flash芯片型号为其他!\r\n");
			Flash_Size = 0;
			break;
		}

		/* 擦除将要写入的 SPI FLASH 扇区，FLASH写入前要先擦除 
		// 这里擦除4K，即一个扇区，擦除的最小单位是扇区
		//SPI_FLASH_SectorErase(FLASH_SectorToErase);
		*/
		/* 将发送缓冲区的数据写到flash中 
		// 这里写一页，一页的大小为256个字节
		SPI_FLASH_BufferWrite(Tx_Buffer, FLASH_WriteAddress, BufferSize);
		printf("\r\n 写入的数据为：%s \r\t", Tx_Buffer);
		*/

		/* 将刚刚写入的数据读出来放到接收缓冲区中 
		SPI_FLASH_BufferRead(Rx_Buffer, FLASH_ReadAddress, BufferSize);
		printf("\r\n 读出的数据为：%s \r\n", Rx_Buffer);
		*/

		/* 检查写入的数据与读出的数据是否相等 
		TransferStatus1 = Buffercmp(Tx_Buffer, Rx_Buffer, BufferSize);

		if (PASSED == TransferStatus1)
		{

			printf("\r\n 串行flash测试成功!\n\r");
		}
		else
		{

			printf("\r\n 串行flash测试失败!\n\r");
		}
		*/
	}// if (FlashID == sFLASH_ID)
	else// if (FlashID == sFLASH_ID)
	{

		printf("\r\n 获取不到 W25Q64 ID!\n\r");
	}



		delay_ms(500);
		//LED = 1;//PC13熄灭	
		delay_ms(500);
	
	while (1)
	{
		Key_Scan();
		delay_ms(10);
		
		if (g_KeyFlag == 1) {
		//LED = 0; //PC13点亮
		//屏幕内容显示	
		LCD_ShowString(5, 0, "ZoomeXplore", RED, WHITE, 16, 0); LCD_ShowChinese(95, 0, "极速探索", RED, WHITE, 16, 0);
		//LCD_ShowString(10, 20, "LCD_W:", RED, WHITE, 16, 0);
		//LCD_ShowIntNum(58, 20, LCD_W, 3, RED, WHITE, 16);
		//LCD_ShowString(10, 40, "LCD_H:", RED, WHITE, 16, 0);
		//LCD_ShowIntNum(58, 40, LCD_H, 3, RED, WHITE, 16);
		LCD_ShowString(10+80, 60, "Flash:", RED, WHITE, 16, 0); LCD_ShowIntNum(71+69, 60, Flash_Size, 1, LIGHTBLUE, WHITE, 16); LCD_ShowString(79+70, 60, "M", LIGHTBLUE, WHITE, 16, 0);
		LCD_ShowPicture(110, 20, 40, 40, gImage_1);
		LCD_ShowPicture(60, 20, 40, 40, gImage_2);
		LCD_ShowPicture(10, 20, 40, 40, gImage_3); //gImage_16080
			LCD_ShowString(2, 60, "KEY RLS.", BLUE, WHITE, 16, 0);
		} else {
			LCD_ShowPicture(0, 0, 160, 80, gImage_16080); //
			LCD_ShowString(2, 60, "KEY CFM.", BLUE, WHITE, 16, 0);
		}
	}
}

TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
	while (BufferLength--)
	{
		if (*pBuffer1 != *pBuffer2)
		{
			return FAILED;
		}

		pBuffer1++;
		pBuffer2++;
	}
	return PASSED;
}
