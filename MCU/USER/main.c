//
//  文 件 名   : main.c
//  版 本 号   :
//  作    者   :
//  生成日期   :
//  最近修改   : 2024-07-05，优化子页面跳转到主页的定位光标处理，增加长按“钥匙”按钮进入屏保功能
//  功能描述   : 
//              STM32F103 TFT开发板系列 
//              STM32F103C8T6
//              京东方玻璃 0.96 inch 80RGB*160DOTS MODULE NUMBER: ZJY096T-IF09 ST7735S Driver
//
//              按键
//              ----------------------------------------------------------------
//              KEY   PA0
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
//              ----------------------------------------------------------------
//
//							FLASH芯片接线
//              ----------------------------------------------------------------
//              GND   电源地
//              VCC   3.3v
//              CS    PB12
//              CLK   PB13
//              DO    PB14
//              DI    PB15
//              ----------------------------------------------------------------
//
//							BMP280接线
//              ----------------------------------------------------------------
//              GND   电源地
//              VCC   3.3V
//              SCL   (PC12)->PB10
//              SDA   (PC11)->PB11
//              
//							mcuisp串口下载，使用串口USART1，管脚PA9、PA10，下载程序选择STMISP下载模式
//							---------------------------------------------------------------
//							热进入:同时按住复位和BOOT按键,先松开复位按键约1S后松开BOOT0按键即进入BOOT下载模式。
//							冷进入:按住BOOT0按键后再接入USB电源约1S后松开BOOT0按键即进入BOOT下载模式。
//
//  片内FLASH及RAM分布
//  ---------------------------------------------------------------------------
//  RAM - RW-data/可读写数据段/被调用的已经初始且初始化不为0的全局数据
//  RAM - ZI-data/清零数据段/被调用的不初始或初始化为0的全局数据
//  FLASH - RO-data只读数据段
//  FLASH - Code代码段
//  ---------------------------------------------------------------------------

#include "delay.h"
#include "sys.h"
#include "led.h"
#include "lcd_init.h"
#include "lcd.h"
#include "pic.h"
#include "usart_1.h"
#include "W25QXX.h"
#include "bmp280.h"

#include "stm32f10x_tim.h"

//单个按键检测
//#define SingleKeyEvent 1
//#ifdef SingleKeyEvent
//#endif

//按键对应的IO管脚 PA.0
#define KEY_IO_RCC	RCC_APB2Periph_GPIOA
#define KEY_IO_PORT	GPIOA
#define KEY_IO_PIN	GPIO_Pin_0
//Key: 1:高电平，按键未按下， 0：低电平，按键按下
#define Key	GPIO_ReadInputDataBit(KEY_IO_PORT, KEY_IO_PIN)

typedef enum
{
    KEY_CHECK = 0,
    KEY_COMFIRM = 1,
    KEY_RELEASE = 2 
}KEY_STATE;

typedef enum 
{
    NULL_KEY = 0,
    SHORT_KEY = 1,
    LONG_KEY = 2
}KEY_TYPE;

KEY_STATE KeyState = KEY_CHECK;	// 初始化按键状态为检测状态
u8 g_KeyFlag = 0;								// 按键有效标志，0： 按键值无效； 1：按键值有效
KEY_TYPE g_KeyActionFlag;		//用于区别长按和短按
char bScreenMain = 1; //是否在主页面
char nSubMenu; //菜单1、2、3、4.0为不在子菜单页面

u8 bGetBMP280Flag = 0; //是否可以获取BMP280数据

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
uint8_t Tx_Buffer[256]; // = "123456";


uint8_t Rx_Buffer[BufferSize];
__IO uint32_t DeviceID = 0;
__IO uint32_t FlashID = 0;
__IO TestStatus TransferStatus1 = FAILED;

// 函数原型声明
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);

void Timer_init(uint16_t per,uint16_t psc);
void Key_Scan(void);
void ScreenMain(void);
void FlashRW(void);
void ScreenMainItem(void);

int Pic2Flash(int nNo, int nPkgNo); //图片数据写入Flash（包括编号、头信息，建议每次写一行像素的数据）

void ShowPicLine(int nNo, int nPkgNo); //显示一行图像数据
void ShowPicFlash(int nNo); //显示指定编号的FLASH上的图片到TFT上
void ShowPicComm(int nNo, int nPkgNo); //显示指定编号的串口传过来上的图片到TFT上

__IO uint32_t Flash_Size = 0;
int nCountSingleClick = 0; //主页面上短按计数
int nCountHoldClick = 0; //长按计数
char bScreenSaver = 0; //是否启用屏保

//大气压传感器
float bmp280_temp;
float bmp280_press;
float high;

long Press, Temp, High;
char PicShowFlag[100] = {0}; //对100个存储区的图片进行标记
int nBufferSize; //接收的数据大小

#define PKGSIZE_DATA 210
int main()
{
	//float t = 0;
	//__IO uint32_t Flash_Size = 0;
	//int i;
	
	delay_init();
	LED_Init();//LED初始化
	Usart1_Init(57600); //(115200);
	SPI_FLASH_Init();

	LCD_Init();//LCD初始化
	//LCD_BLK_Set();//打开背光
	LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
	
	Timer_init(19, 7199); //10Khz的计数频率，计数到20为2ms
	////bmp280
//	float bmp280_temp;
//	float bmp280_press;
//	float high;
	//unsigned short Press,Temp,High;
	bmp280Init();
	
	bmp280GetData(&bmp280_press,&bmp280_temp,&high);
		delay_ms(500);
		Press = bmp280_press * 1000;
		Temp = bmp280_temp * 1000;
		High = high * 1000;
		printf("bmp280_press:%d\r\n",Press);
		delay_ms(100);
		printf("bmp280_temp :%d\r\n",Temp);
		delay_ms(100);
		printf("bmp280_high :%d\r\n\r\n",High);
		delay_ms(100);
	
	delay_ms(500);
	bmp280GetData(&bmp280_press,&bmp280_temp,&high);
		delay_ms(500);
		Press = bmp280_press * 1000;
		Temp = bmp280_temp * 1000;
		High = high * 1000;
		printf("bmp280_press2:%d\r\n",Press);
		delay_ms(100);
		printf("bmp280_temp2 :%d\r\n",Temp);
		delay_ms(100);
		printf("bmp280_high2 :%d\r\n",High);
		delay_ms(100);
	////
	LED = 0;
	PicShowFlag[63] = 1; //W25Q保存的图片标记
	PicShowFlag[74] = 1; 
	PicShowFlag[85] = 1; 
	PicShowFlag[99] = 1; 
	/* 获取 Flash Device ID */
	DeviceID = SPI_FLASH_ReadDeviceID();

	/* 获取 SPI Flash ID */
	FlashID = SPI_FLASH_ReadID();
	printf("\r\nFlashID is 0x%X \r\nDevice ID is 0x%X\r\n", FlashID, DeviceID);

	/* 检验 SPI Flash ID */
	if (FlashID == W25Q16ID || W25Q32ID || W25Q64ID || W25Q80ID)
	{
		printf("\r\n检测到串行flash芯片!\r\n");
		switch (FlashID)
		{
		case W25Q16ID:
			printf("\r\nflash芯片型号为W25Q16ID!\r\n");
			Flash_Size = 2;
			break;
		case W25Q32ID:
			printf("\r\nflash芯片型号为W25Q32!\r\n");
			Flash_Size = 4;
			break;
		case W25Q64ID:
			printf("\r\nflash芯片型号为W25Q64!\r\n");
			Flash_Size = 8;
			break;
		case W25Q80ID:
			printf("\r\nflash芯片型号为W25Q80!\r\n");
			Flash_Size = 1;
			break;
		default:
			printf("\r\nflash芯片型号为其他!\r\n");
			Flash_Size = 0;
			break;
		}
		
		//FlashRW();//FLASH读写测试
		
	} else {
		printf("\r\n获取不到 W25Q64 ID!\n\r");
	}

	delay_ms(500);
	LED = 1;//PC13熄灭
	//delay_ms(500);
	//LCD_BLK_Set();
	//LCD_BLK_Clr();
	//BIT_ADDR(GPIOB_ODR_Addr,8) = 1;
	//PBout(8) = 0;
	ScreenMain();
	
	while (1)
	{
		if(usart1.RecFlag == 1) {
			int bRetFlashSuc = 1;
			//printf("\r\nZXI> Download data to W25Q32, Len:%d, Adr:0x%08X\r\n", usart1.RecLen, usart1.AdrPrgflash);
			//LCD_ShowChinese(2, 12, usart1.RxBuff, LBBLUE, WHITE, 16, 0);
			if (Buffercmp(usart1.RxBuff, "ZXI_HEAD", 8)) {
				printf("ZXI>IMG HEAD, No.:%03d, Width:%d, High:%d\r\n", 
					usart1.RxBuff[8], usart1.RxBuff[11] | (usart1.RxBuff[10] >> 8), usart1.RxBuff[13] | (usart1.RxBuff[12] >> 8));
				LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
				unsigned char tmpRxBuf[20];
				memcpy(tmpRxBuf, usart1.RxBuff, 8);
				
				LCD_ShowString(2, 0, tmpRxBuf, GRAYBLUE, WHITE, 16, 0);
				LCD_ShowString(2, 16, "No.  W    H    ", GRAYBLUE, WHITE, 12, 0);
				LCD_ShowIntNumLeadingZeros(2, 28, usart1.RxBuff[8], 3, GRAYBLUE, WHITE, 12); //No
				LCD_ShowIntNumLeadingZeros(2 + 5 * 6, 28, usart1.RxBuff[10] | (usart1.RxBuff[11] >> 8), 4, GRAYBLUE, WHITE, 12); //Width
				LCD_ShowIntNumLeadingZeros(2 + 10 * 6, 28, usart1.RxBuff[12] | (usart1.RxBuff[13] >> 8), 4, GRAYBLUE, WHITE, 12); //High
				//写FLASH
				for (int i = 0; i < 14; i++) {
					Tx_Buffer[i] = usart1.RxBuff[i];
				}
				Tx_Buffer[14] = '\0';
				bool bRet = Pic2Flash(usart1.RxBuff[8], 0);
				if (bRet == 0) {
					printf("ZXI>IMG HEAD Write Flash Suc.(%s)\r\n\r\n", Tx_Buffer);
				} else {
					printf("ZXI>IMG HEAD Write Flash Fail.\r\n");
				}
			} else if (Buffercmp(usart1.RxBuff, "ZXI_DATA", 8)) {
				printf("ZXI>IMG DATA, No.:%03d, PackageNo:%d\r\n", usart1.RxBuff[8], usart1.RxBuff[9]);
				memcpy(Tx_Buffer, usart1.RxBuff, nBufferSize);
				LCD_ShowString(2, 48, "DATA:" , GRAYBLUE, WHITE, 16, 0);
				
				LCD_ShowString(2 + 5 * 8, 42, "No. PkgNo" , GRAYBLUE, WHITE, 12, 0);
				LCD_ShowIntNumLeadingZeros(2 + 5 * 8, 54, usart1.RxBuff[8], 3, GRAYBLUE, WHITE, 12); //No
				LCD_ShowIntNumLeadingZeros(2 + 5 * 8 + 4 * 6, 54, usart1.RxBuff[9], 3, GRAYBLUE, WHITE, 12); //PackageNo
				
				LCD_ShowString(2 + 9 * 8 + 4 * 8, 48, "0x" , GRAYBLUE, WHITE, 12, 0);
				LCD_ShowHEX(2 + 9 * 8 + 4 * 8 + 2 * 6 + 2, 48, usart1.RxBuff[10], GRAYBLUE, WHITE, 12); //rgb data
				LCD_ShowHEX(2 + 9 * 8 + 4 * 8 + 4 * 6 + 4, 48, usart1.RxBuff[11], GRAYBLUE, WHITE, 12); //rgb data
				LCD_ShowHEX(2 + 9 * 8 + 4 * 8 + 6 * 6 + 6, 48, usart1.RxBuff[12], GRAYBLUE, WHITE, 12); //rgb data
				
				for (int i = 0; i < PKGSIZE_DATA; i++) {
					Tx_Buffer[i] = usart1.RxBuff[i];
				}
				Tx_Buffer[PKGSIZE_DATA] = '\0';
				bool bRet = Pic2Flash(usart1.RxBuff[8], usart1.RxBuff[9]);
				if (bRet == 0) {
					bRetFlashSuc = bRet;
					printf("ZXI>IMG DATA Write Flash Suc.(%s)\r\n\r\n", Tx_Buffer);
				} else {
					printf("ZXI>IMG DATA Write Flash Fail.\r\n");
				}
			}

			usart1.RecFlag = 0;
			usart1.RecLen = 0;
			
			if (usart1.RxBuff[9] == 128) {
				ScreenMain(); //返回主页面
				bScreenMain = 1;
				nSubMenu = 0;
			}
		}
		if (bGetBMP280Flag == 1) {
			bGetBMP280Flag = 0;
			Press = bmp280_press * 1000;
			Temp = bmp280_temp * 1000;
			High = high * 1000;
			//printf("Press:%ld\r\n", Press);
			//printf("bmp280_press:%.4f\r\n", bmp280_press);
			//printf("bmp280_temp :%.2f\r\n", bmp280_temp);
			//printf("bmp280_high :%.1f\r\n\r\n", high); 
			
			if (bScreenMain == 1 && bScreenSaver == 0) {
				//温度
				LCD_ShowString(2, 65, "T:", BLUE, WHITE, 12, 0);
				;
				if (bmp280_temp < 0) {
					LCD_ShowString(2 + 12, 65, "-", BLUE, WHITE, 12, 0);
					LCD_ShowFloatNum1(2 + 12 + 6, 65, -bmp280_temp, 4, GRAYBLUE, WHITE, 12);
					LCD_ShowString(2 + 12 + 5 * 6 + 6, 65, "C", BLUE, WHITE, 12, 0);
				} else {
					LCD_ShowFloatNum1(2 + 12, 65, bmp280_temp, 4, GRAYBLUE, WHITE, 12);
					LCD_ShowString(2 + 12 + 5 * 6, 65, "C", BLUE, WHITE, 12, 0);
				}

				//大气压
				LCD_ShowString(2 + 12 + 6 * 6,      65, " P:", BLUE, WHITE, 12, 0);
        LCD_ShowIntNumLeadingZeros(2 + 12 + 6 * 6 + 3 * 6, 65, (int)bmp280_press, 4, GRAYBLUE, WHITE, 12);
				LCD_ShowFloatNum1(         2 + 12 + 6 * 6 + 3 * 6 + 4 * 6, 65, (bmp280_press - (int)bmp280_press) * 100, 4, GRAYBLUE, WHITE, 12);
				LCD_ShowString(            2 + 12 + 6 * 6 + 3 * 6 + 4 * 6 + 5 * 6, 65, "Pa", BLUE, WHITE, 12, 0);
				//海拔
				LCD_ShowIntNumLeadingZeros(2 + 12 + 6 * 6 + 3 * 6 + 4 * 6 + 5 * 6 + 2 * 6, 65, high, 4, GRAYBLUE, WHITE, 12);
				//LCD_ShowFloatNum1(         2 + 12 + 6 * 6 + 3 * 6 + 4 * 6 + 5 * 6 + 2 * 6, 65, high / 100, 3, GRAYBLUE, WHITE, 12);

			//} else if (nSubMenu == 3) { //module info

			} else if(nSubMenu == 2) { //在BMP280信息页面
	  		//温度
				//bmp280_temp -= 40;
  			LCD_ShowChinese(2, 12, "温度：", LBBLUE, WHITE, 16, 0);
				//LCD_ShowString(12, 16, "TEMP:", BLUE, WHITE, 16, 0);
				if (bmp280_temp < 0) {
					LCD_ShowString(2 + 6 * 8, 6, "-", GRAYBLUE, WHITE, 24, 0);
					LCD_ShowFloatNum1(2 + 6 * 8 + 12, 6, -bmp280_temp, 4, GRAYBLUE, WHITE, 24);
					LCD_ShowChinese(2 + 6 * 8 + 6 * 12, 10, "℃", GRAYBLUE, WHITE, 16, 0);			
				} else {
					LCD_ShowFloatNum1(2 + 6 * 8, 6, bmp280_temp, 4, GRAYBLUE, WHITE, 24);
					LCD_ShowChinese(2 + 6 * 8 + 5 * 12, 10, "℃", GRAYBLUE, WHITE, 16, 0);
				}
				//大气压
				LCD_ShowChinese(0, 32, "大气压：", LBBLUE, WHITE, 16, 0);
				//LCD_ShowString(12, 32, "PRESS:", BLUE, WHITE, 16, 0);
				LCD_ShowIntNumLeadingZeros(0 + 8 * 8, 32, (int)bmp280_press, 4, GRAYBLUE, WHITE, 16);
				LCD_ShowString(0 + 8 * 8 + 4 * 8, 32, ".", BLUE, WHITE, 16, 0);
				LCD_ShowIntNumLeadingZeros(0 + 8 * 8 + 4 * 8 + 1 * 8, 32, (bmp280_press - (int)bmp280_press) * 10000, 4, GRAYBLUE, WHITE, 16);
				LCD_ShowString(0 + 8 * 8 + 4 * 8 + 1 * 8 + 4 * 8, 32, "hPa", GRAYBLUE, WHITE, 16, 0);

				//海拔
				LCD_ShowChinese(2, 52, "海拔：", LBBLUE, WHITE, 16, 0);
				//LCD_ShowString(12, 48, "ASL:", BLUE, WHITE, 16, 0);
				LCD_ShowIntNum(2 + 6 * 8, 52, (int)high, 7, GRAYBLUE, WHITE, 16);
				//LCD_ShowString(2 + 6 * 8 + 7 * 8, 52, "CM", GRAYBLUE, WHITE, 16, 0);
			}
		}
		switch(g_KeyActionFlag)
    {
			case SHORT_KEY:
				//执行短按对应的事件
			  printf("KEY: SINGLE CLICK.\r\n");
				if(bScreenMain == 0) { //从子页面返回
					ScreenMain();
					bScreenMain = 1;
					nSubMenu = 0;
				} else { //在主页面
					if (bScreenSaver == 1) { //解除屏保
						bScreenSaver = 0;
						ScreenMain();
					}
					nCountSingleClick++;
					nCountSingleClick = nCountSingleClick > 3 ? 0 : nCountSingleClick;

					LCD_Fill(0, 25 + 34, LCD_W, 25 + 36 + 6, WHITE); //清除gImage_x下指示^

					ScreenMainItem();
					//nCountSingleClick++;
				}
				g_KeyActionFlag = NULL_KEY; //(KEY_TYPE)0;		//状态回到空
				break;

			case LONG_KEY:
				//执行长按对应的事件
			  printf("KEY: HOLD CLICK.\r\n");
				//LCD_ShowString(2, 60, "KEY PRESS LONG ", BLUE, WHITE, 16, 0);
				if (nCountSingleClick == 1 && bScreenMain == 1) { //BMP280 info
					bScreenMain = 0; //标记为非主页面
					nSubMenu = 2;
					LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
				} else if (nCountSingleClick == 2 && bScreenMain == 1) { //module info
					bScreenMain = 0; //标记为非主页面
					//nSubMenu = 3;
					LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
					LCD_ShowString(0,  0, "MCU: STM32F103C8T6", LBBLUE, WHITE, 16, 0);
					LCD_ShowString(0, 16, "IPS: 0.96' 80RGB*160", LBBLUE, WHITE, 16, 0);
					LCD_ShowString(0, 32, "LCD Driver: ST7735S", LBBLUE, WHITE, 16, 0);
					LCD_ShowString(0, 48, "Pressure Sen: BMP280", LBBLUE, WHITE, 16, 0);
					LCD_ShowString(0, 64, "Power SOC: IP5306", LBBLUE, WHITE, 16, 0);
				} else if (nCountSingleClick == 3 && bScreenMain == 1) { //在主页面的"钥匙"按钮上长按
					bScreenSaver = 1;
					LCD_Fill(0, 0, LCD_W, LCD_H, BLACK); //全屏黑色填充
				} else {
					LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);

					nCountHoldClick = nCountHoldClick > 100 ? 0 : nCountHoldClick;
					if(nCountHoldClick == 0) {
						LCD_ShowPicture(50, 10, 52, 60, gImage_1_8080);
					} else if (nCountHoldClick == 1) {
						//LCD_ShowString(44, 28 - 6, "Page 2", GRAY, WHITE, 24, 0);
						for (int i = 0, j = 0; i < 5; i++) {
							
							for (int k = 0; k < 20; k++) {
								if (PicShowFlag[k + j * 20] == 1)
									LCD_ShowString(0 + k * 8, j * 16, "#", RED, WHITE, 16, 1);
								else
									LCD_ShowString(0 + k * 8, j * 16, "*", GRAYBLUE, WHITE, 16, 1);
							}
							if (j < 5) j++;
						}
					} else if (nCountHoldClick == 2) {
						LCD_ShowString(8, 0, "Then, My Upload Pic", LBBLUE, WHITE, 16, 1);
						LCD_ShowString(9, 0, "Then, My Upload Pic", LBBLUE, WHITE, 16, 1);//叠加加粗
						LCD_ShowString(8, 16, "Width:160DOT", BLACK, WHITE, 16, 0);
						LCD_ShowString(8, 32, "High:80DOT", BLACK, WHITE, 16, 0);
						LCD_ShowString(8, 48, "Color:16bits(565)", BLACK, WHITE, 16, 0);
						LCD_ShowString(8, 64, "MSB First:Yes", BLACK, WHITE, 16, 0);
					} else {
						ShowPicFlash(nCountHoldClick);
					}
					nCountHoldClick++;

					bScreenMain = 0; //标记为非主页面
				}
				g_KeyActionFlag = NULL_KEY; //(KEY_TYPE)0;		//状态回到空
				break;
			
			default:
				break;
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

void FlashRW()
{
		/* 擦除将要写入的 SPI FLASH 扇区，FLASH写入前要先擦除 */
		// 这里擦除4K，即一个扇区，擦除的最小单位是扇区
		SPI_FLASH_SectorErase(FLASH_SectorToErase);
		
		/* 将发送缓冲区的数据写到flash中 */
		// 这里写一页，一页的大小为256个字节
		SPI_FLASH_BufferWrite(Tx_Buffer, FLASH_WriteAddress, BufferSize);
		printf("\r\n 写入的数据为：%s \r\t", Tx_Buffer);
		
		/* 将刚刚写入的数据读出来放到接收缓冲区中 */
		SPI_FLASH_BufferRead(Rx_Buffer, FLASH_ReadAddress, BufferSize);
		printf("\r\n 读出的数据为：%s \r\n", Rx_Buffer);
		
		/* 检查写入的数据与读出的数据是否相等 */
		TransferStatus1 = Buffercmp(Tx_Buffer, Rx_Buffer, BufferSize);

		if (PASSED == TransferStatus1) {
			printf("\r\n 串行flash测试成功!\n\r");
		} else {
			printf("\r\n 串行flash测试失败!\n\r");
		}
}

void ScreenMain()
{
	LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);

	LCD_ShowString(5, 0, "ZoomeXplore.cn", RED, WHITE, 16, 0); //LCD_ShowChinese(95+24, 0, "探索", RED, WHITE, 16, 0);
	LCD_ShowPicture(160 - 16, 0, 16, 16, gImage_logo16x16); //logo
	//LCD_ShowString(10, 20, "LCD_W:", RED, WHITE, 16, 0);
	//LCD_ShowIntNum(58, 20, LCD_W, 3, RED, WHITE, 16);
	//LCD_ShowString(10, 40, "LCD_H:", RED, WHITE, 16, 0);
	//LCD_ShowIntNum(58, 40, LCD_H, 3, RED, WHITE, 16);
	LCD_ShowPicture(140, 25, 16, 32, gImage_4); //钥匙图标、黑屏
	LCD_ShowPicture(95, 18, 36, 40, gImage_3);  //系统信息
	LCD_ShowPicture(45, 30, 40, 28, gImage_2);  //BMP-280温度、大气压数据
	LCD_ShowPicture(5,  20, 33, 38, gImage_1);
	
	ScreenMainItem();
  //LCD_ShowString(2, 65, "SINGLE/HOLD CLICK", BLUE, WHITE, 12, 0);
	//LCD_ShowString(115, 65, "ROM:", GRAYBLUE, WHITE, 12, 0); 
	LCD_ShowIntNum(73+70, 65, Flash_Size, 1, GRAYBLUE, WHITE, 12); LCD_ShowString(79+70, 65, "M", GRAYBLUE, WHITE, 12, 0);
}

void ScreenMainItem(void)
{
	LCD_ShowString(20 + nCountSingleClick * 40 + 3, 25 + 33, "^", RED, WHITE, 16, 1);//gImage_x下指示
	LCD_ShowString(20 + nCountSingleClick * 40 + 3, 25 + 34, "^", RED, WHITE, 16, 1);
	
	LCD_DrawRectangle(5 - 1, 20 + 38, 5 + 33 + 1, 20 + 38 + 1, WHITE);
	LCD_DrawRectangle(45 - 1, 30 + 28, 45 + 40 + 1, 30 + 28 + 1, WHITE);
	LCD_DrawRectangle(95 - 1, 18 + 40, 95 + 36 + 1, 18 + 40 + 1, WHITE);
	LCD_DrawRectangle(140 - 1, 25 + 32, 140 + 16 + 1, 25 + 32 + 1, WHITE);
	if (nCountSingleClick == 0) LCD_DrawRectangle(5 - 1, 20 + 38, 5 + 33 + 1, 20 + 38 + 1, RED);          //(5 - 1, 20 - 1, 5 + 33 + 1, 20 + 38 + 1, RED);
	else if (nCountSingleClick == 1) LCD_DrawRectangle(45 - 1, 30 + 28, 45 + 40 + 1, 30 + 28 + 1, RED);   //(45 - 1, 30 - 1, 45 + 40 + 1, 30 + 28 + 1, RED);
	else if (nCountSingleClick == 2) LCD_DrawRectangle(95 - 1, 18 + 40, 95 + 36 + 1, 18 + 40 + 1, RED);   //(95 - 1, 18 - 1, 95 + 36 + 1, 18 + 40 + 1, RED);
	else if (nCountSingleClick == 3) LCD_DrawRectangle(140 - 1, 25 + 32, 140 + 16 + 1, 25 + 32 + 1, RED); //(140 - 1, 25 - 1, 140 + 16 + 1, 25 + 32 + 1, RED);
}

void Timer_init(uint16_t per,uint16_t psc)
{
	//库函数版本
	//----------------------------------------------------------------//
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = per;
	TIM_TimeBaseInitStruct.TIM_Prescaler = psc;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	TIM_Cmd(TIM3,ENABLE);
	//----------------------------------------------------------------//
	
	//寄存器版本
	//----------------------------------------------------------------//
//	RCC->APB1ENR |= (uint32_t)0x00000002; //打开定时器3的时钟
//	TIM3->CR1 &= (uint16_t)0xFF8F; //CMS[1:0]和DIR位清零
//	TIM3->CR1 |= (uint16_t)0x0000;   //设置计数为向上计数
//	
//	TIM3->CR1 &= (uint16_t)0xFCFF; //CKD[1:0]位清零
//	TIM3->CR1 |= (uint16_t)0x0000;   //设置1分频
//	
//	TIM3->ARR =  9999; //设置自动装载值
//	TIM3->PSC =  7199; //设置预分频值
//	
//	TIM3->EGR = (uint16_t)0x0001;
//	
//	TIM3->DIER |= (uint16_t)0x0001;
//	
//  my_tmppriority = (0x700 - ((SCB->AIRCR) & (uint32_t)0x700))>> 0x08;
//  my_tmppre = (0x4 - my_tmppriority);
//  my_tmpsub = my_tmpsub >> my_tmppriority;

//  my_tmppriority = 1 << my_tmppre; //抢占优先级为1
//  my_tmppriority |=  2 & my_tmpsub;//子优先级为2
//  my_tmppriority = my_tmppriority << 0x04;
//	
//	NVIC->IP[TIM3_IRQn] = my_tmppriority;
//	NVIC->ISER[TIM3_IRQn >> 0x05] =(uint32_t)0x01 << (TIM3_IRQn & (uint8_t)0x1F);//开启通道
//	//NVIC->ISER[0] =1<<29;
//	TIM3->CR1 |= (uint16_t)0x0001;
}

/**
 * 单个按键检测短按和长按事件
 * 短按：时间 10ms < T < 1 s, 长按：时间 T >1 s
 * 功能：使用状态机方式，扫描单个按键；扫描周期为10ms,10ms刚好跳过抖动；
 * 状态机使用switch case语句实现状态之间的跳转
 * lock变量用于判断是否是第一次进行按键确认状态
 * ：按键释放后才执行按键事件
 */
void Key_Scan0(void)
{
    static u8 TimeCnt = 0;
    static u8 lock = 0;
    switch(KeyState)
    {
        //按键未按下状态，此时判断Key的值
        case KEY_CHECK:    
					if(Key)   //!Key
					{
						KeyState =  KEY_COMFIRM;  //如果按键Key值为0，说明按键开始按下，进入下一个状态 
					}
					TimeCnt = 0;                  //计数复位
					lock = 0;
					break;
            
        case KEY_COMFIRM:
					if(Key)  //!Key                   //查看当前Key是否还是0，再次确认是否按下
					{
						if(!lock)   lock = 1;
						TimeCnt++;
					}   
					else                       
					{
							if(lock)                // 不是第一次进入，  释放按键才执行
							{
									//按键时长判断
									if(TimeCnt > 100)            // 长按 1 s
									{
											g_KeyActionFlag = LONG_KEY;
											TimeCnt = 0;  
									}
									else                         // Key值变为了1，说明此处动作为短按
									{
											g_KeyActionFlag = SHORT_KEY;          // 短按
									}
									
									KeyState =  KEY_RELEASE;    // 需要进入按键释放状态 
							}
							else                          // 当前Key值为1，确认为抖动，则返回上一个状态
							{
									KeyState =  KEY_CHECK;    // 返回上一个状态
							}
					} 
          break;
            
        case KEY_RELEASE:
          if(!Key) //Key                    //当前Key值为1，说明按键已经释放，返回开始状态
          { 
            KeyState =  KEY_CHECK;    
          }
          break;
             
        default: 
					break;
    }    
}

/**
 * 单个按键检测短按和长按事件
 * 短按：时间 10ms < T < 1 s, 长按：时间 T >1 s
 * 功能：使用状态机方式，扫描单个按键；扫描周期为10ms,10ms刚好跳过抖动；
 * 状态机使用switch case语句实现状态之间的跳转
 * lock变量用于判断是否是第一次进行按键确认状态
 * 长按键事件提前执行，短按键事件释放后才执行
 */
//u8 TimeCnt = 0;
//u8 lock = 0;
void Key_Scan(void)
{
		static u8 TimeCnt = 0;
    static u8 lock = 0;
	
    switch (KeyState)
    {
        //按键未按下状态，此时判断Key的值
        case KEY_CHECK:    
           if(Key)   //!Key 
            {
							//printf("KeyState=KEY_CHECK !Key\r\n");
              KeyState = KEY_COMFIRM;  //如果按键Key值为0，说明按键开始按下，进入下一个状态 
            }
            TimeCnt = 0;                  //计数复位
            lock = 0;
            break;
            
        case KEY_COMFIRM:
            if(Key)  //!Key                   //查看当前Key是否还是0，再次确认是否按下
            {
              if(!lock) lock = 1;
               	
							//printf("KeyState=KEY_COMFIRM !Key\r\n");

              TimeCnt++;  
                
              //按键时长判断
              if(TimeCnt > 100)            // 长按 1 s
              {
                g_KeyActionFlag = LONG_KEY;
                TimeCnt = 0;  
                lock = 0;               //重新检查
                KeyState = KEY_RELEASE;    // 需要进入按键释放状态
              }
            }   
            else                       
            {
              if(1 == lock)                // 不是第一次进入，  释放按键才执行
              {
                g_KeyActionFlag = SHORT_KEY;          // 短按
                KeyState = KEY_RELEASE;    // 需要进入按键释放状态 
              }
              else                          // 当前Key值为1，确认为抖动，则返回上一个状态
              {
                KeyState = KEY_CHECK;    // 返回上一个状态
              }
            } 
            break;
            
         case KEY_RELEASE:
             if(!Key)   //Key                  //当前Key值为1，说明按键已经释放，返回开始状态
             { 
                 KeyState = KEY_CHECK;    
             }
						 break;
             
         default: 
					 break;
    }    
}

void TIM3_IRQHandler(void)   //TIM3 每 2ms 中断一次
{   
	static u8 cnt;
	static u16 cnt_bmp280; //	1000ms取一次BMP280数据

	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源    
	{
		cnt++;
		cnt_bmp280++;
		
    if(cnt > 5)	//每10ms 执行一次按键扫描程序
    {
			Key_Scan();
			cnt = 0;
			//printf("TIM3 IRQ\r\n");
    }

		if (cnt_bmp280 > 500) {
			bmp280GetData(&bmp280_press, &bmp280_temp, &high);
			bGetBMP280Flag = 1;
			cnt_bmp280 = 0;
		}
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源
	}
}

//图片数据写入Flash（包括编号、头信息，建议每次写半行像素的数据）
//nPkgNo为0时为数据头，    包括协议头"ZXI_HEAD"(8BYTE)+图像编号(1BYTE)+包编号0(1BYTE)+宽度MSB(1BYTE)+宽度LSB(1BYTE)+高度MSB(1BYTE)+高度LSB(1BYTE)信息
//nPkgNo>0时为图像RGB数据，包括协议头"ZXI_DATA"(8BYTE)+图像编号(1BYTE)+包编号N(1BYTE)+RGB数据(n BYTE)，每次传200字节图像有效信息，最后一包不足200用0x00填充
int Pic2Flash(int nNo, int nPkgNo)
{
	u32 nAdrStart = 0x00000;
	
	nAdrStart = nAdrStart + (nNo - 1) * 4 * 1024 * 8; //每个编号的数据分配160*80*2=25600<4*1024*8=32768字节的空间
	//return 1;

	//擦除将要写入的 SPI FLASH 扇区，FLASH写入前要先擦除
	//这里擦除4K，即一个扇区，擦除的最小单位是扇区,一个图片占用32K空间
	if (nPkgNo == 0) {
		SPI_FLASH_SectorErase(nAdrStart);
		SPI_FLASH_SectorErase(nAdrStart + 1 * 4096);
		SPI_FLASH_SectorErase(nAdrStart + 2 * 4096);
		SPI_FLASH_SectorErase(nAdrStart + 3 * 4096);
		SPI_FLASH_SectorErase(nAdrStart + 4 * 4096);
		SPI_FLASH_SectorErase(nAdrStart + 5 * 4096);
		SPI_FLASH_SectorErase(nAdrStart + 6 * 4096);
		SPI_FLASH_SectorErase(nAdrStart + 7 * 4096);
	}
	//将发送缓冲区的数据写到flash中
	//这里写一页，一页的大小为256个字节,实际写入208+2字节,W25Q利用率大约78%
	nAdrStart += nPkgNo * 256;
	nBufferSize = (nPkgNo == 0) ? 14 : PKGSIZE_DATA;
	SPI_FLASH_BufferWrite(Tx_Buffer, nAdrStart, nBufferSize);
	
	if (nPkgNo == 1) {
	printf("ZXI>写入的数据为(%d): \r\n", nBufferSize);
	for (int i = 0; i < nBufferSize; i++) {
		printf("%02X ", Tx_Buffer[i]);
	}

	printf("\r\n------------\r\n");
	//将刚刚写入的数据读出来放到接收缓冲区中
	SPI_FLASH_BufferRead(Rx_Buffer, nAdrStart, nBufferSize);
	printf("ZXI>读出的数据为(%d) \r\n", nBufferSize);
	for (int i = 0; i < nBufferSize; i++) {
		printf("%02X ", Rx_Buffer[i]);
	}
	printf("\r\n------------\r\n");
	//检查写入的数据与读出的数据是否相等
	TransferStatus1 = Buffercmp(Tx_Buffer, Rx_Buffer, nBufferSize);

	if (PASSED == TransferStatus1) {
		printf("ZXI>Addr:0x%08X,串行flash写读成功!\r\n", nAdrStart);
		return 0; //写入成功返回0，不是PASSED
	} else {
		printf("ZXI>串行flash写读失败!\r\n");
		return 1; //写入失败返回1，不是FAILED
	}
	
	return 0;} 
	
	return 0;
}

//显示指定编号的FLASH上的图片到TFT上
void ShowPicFlash(int nNo)
{
	//LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
	LCD_Address_Set(0, 0, 159, 79);
	for (int r = 1; r < 129; r++) {
		ShowPicLine(nCountHoldClick, r);
	}
	LCD_ShowIntNumLeadingZeros(8, 0, nCountHoldClick - 2, 3, GRAYBLUE, WHITE, 16);
}

//显示一行图像数据
void ShowPicLine(int nNo, int nPkgNo)
{
	u32 nAdrStart = 0x00000;
	int nBufferSize = 256;
	uint8_t Rx_Buffer_Pic[200];
	u32 k = 0;
	nAdrStart = nAdrStart + (nNo - 1) * 4 * 1024 * 8;

	//LCD_Address_Set(0, 0, 0 + 160 - 1, 0 + 80 - 1);
	SPI_FLASH_BufferRead(Rx_Buffer, nAdrStart + nPkgNo * 256, nBufferSize);
	
	for(int i = 0; i < 200; i++) {
		Rx_Buffer_Pic[i] = Rx_Buffer[ i + 10];
	}
	
	for(int j = 0; j < 100; j++)
	{
		if (nPkgNo == 1) {
			LCD_WR_DATA8(0x00);
			LCD_WR_DATA8(0x00);
		} else {
			LCD_WR_DATA8(Rx_Buffer_Pic[k * 2 + 1]);
			LCD_WR_DATA8(Rx_Buffer_Pic[k * 2]);
		}
		k++;
	}
}
//显示指定编号的串口传过来上的图片到TFT上
void ShowPicComm(int nNo, int nPkgNo)
{
	//int nNo = 3;
}
