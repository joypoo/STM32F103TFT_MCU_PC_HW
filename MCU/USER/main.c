//
//  �� �� ��   : main.c
//  �� �� ��   :
//  ��    ��   :
//  ��������   :
//  ����޸�   : 2024-07-05���Ż���ҳ����ת����ҳ�Ķ�λ��괦�����ӳ�����Կ�ס���ť������������
//  ��������   : 
//              STM32F103 TFT������ϵ�� 
//              STM32F103C8T6
//              ���������� 0.96 inch 80RGB*160DOTS MODULE NUMBER: ZJY096T-IF09 ST7735S Driver
//
//              ����
//              ----------------------------------------------------------------
//              KEY   PA0
//              
//							��Ļ����
//              ----------------------------------------------------------------
//              GND   ��Դ��
//              VCC   3.3v��Դ
//              SCL   PB3��SCLK��
//              SDA   PB5��MOSI��
//              RES   PB6
//              DC    PB4
//              CS    PB7
//              ----------------------------------------------------------------
//
//							FLASHоƬ����
//              ----------------------------------------------------------------
//              GND   ��Դ��
//              VCC   3.3v
//              CS    PB12
//              CLK   PB13
//              DO    PB14
//              DI    PB15
//              ----------------------------------------------------------------
//
//							BMP280����
//              ----------------------------------------------------------------
//              GND   ��Դ��
//              VCC   3.3V
//              SCL   (PC12)->PB10
//              SDA   (PC11)->PB11
//              
//							mcuisp�������أ�ʹ�ô���USART1���ܽ�PA9��PA10�����س���ѡ��STMISP����ģʽ
//							---------------------------------------------------------------
//							�Ƚ���:ͬʱ��ס��λ��BOOT����,���ɿ���λ����Լ1S���ɿ�BOOT0����������BOOT����ģʽ��
//							�����:��סBOOT0�������ٽ���USB��ԴԼ1S���ɿ�BOOT0����������BOOT����ģʽ��
//
//  Ƭ��FLASH��RAM�ֲ�
//  ---------------------------------------------------------------------------
//  RAM - RW-data/�ɶ�д���ݶ�/�����õ��Ѿ���ʼ�ҳ�ʼ����Ϊ0��ȫ������
//  RAM - ZI-data/�������ݶ�/�����õĲ���ʼ���ʼ��Ϊ0��ȫ������
//  FLASH - RO-dataֻ�����ݶ�
//  FLASH - Code�����
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

//�����������
//#define SingleKeyEvent 1
//#ifdef SingleKeyEvent
//#endif

//������Ӧ��IO�ܽ� PA.0
#define KEY_IO_RCC	RCC_APB2Periph_GPIOA
#define KEY_IO_PORT	GPIOA
#define KEY_IO_PIN	GPIO_Pin_0
//Key: 1:�ߵ�ƽ������δ���£� 0���͵�ƽ����������
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

KEY_STATE KeyState = KEY_CHECK;	// ��ʼ������״̬Ϊ���״̬
u8 g_KeyFlag = 0;								// ������Ч��־��0�� ����ֵ��Ч�� 1������ֵ��Ч
KEY_TYPE g_KeyActionFlag;		//�������𳤰��Ͷ̰�
char bScreenMain = 1; //�Ƿ�����ҳ��
char nSubMenu; //�˵�1��2��3��4.0Ϊ�����Ӳ˵�ҳ��

u8 bGetBMP280Flag = 0; //�Ƿ���Ի�ȡBMP280����

//-----------------

typedef enum { FAILED = 0, PASSED = !FAILED } TestStatus;

/* ��ȡ�������ĳ��� */
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

// ����ԭ������
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);

void Timer_init(uint16_t per,uint16_t psc);
void Key_Scan(void);
void ScreenMain(void);
void FlashRW(void);
void ScreenMainItem(void);

int Pic2Flash(int nNo, int nPkgNo); //ͼƬ����д��Flash��������š�ͷ��Ϣ������ÿ��дһ�����ص����ݣ�

void ShowPicLine(int nNo, int nPkgNo); //��ʾһ��ͼ������
void ShowPicFlash(int nNo); //��ʾָ����ŵ�FLASH�ϵ�ͼƬ��TFT��
void ShowPicComm(int nNo, int nPkgNo); //��ʾָ����ŵĴ��ڴ������ϵ�ͼƬ��TFT��

__IO uint32_t Flash_Size = 0;
int nCountSingleClick = 0; //��ҳ���϶̰�����
int nCountHoldClick = 0; //��������
char bScreenSaver = 0; //�Ƿ���������

//����ѹ������
float bmp280_temp;
float bmp280_press;
float high;

long Press, Temp, High;
char PicShowFlag[100] = {0}; //��100���洢����ͼƬ���б��
int nBufferSize; //���յ����ݴ�С

#define PKGSIZE_DATA 210
int main()
{
	//float t = 0;
	//__IO uint32_t Flash_Size = 0;
	//int i;
	
	delay_init();
	LED_Init();//LED��ʼ��
	Usart1_Init(57600); //(115200);
	SPI_FLASH_Init();

	LCD_Init();//LCD��ʼ��
	//LCD_BLK_Set();//�򿪱���
	LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
	
	Timer_init(19, 7199); //10Khz�ļ���Ƶ�ʣ�������20Ϊ2ms
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
	PicShowFlag[63] = 1; //W25Q�����ͼƬ���
	PicShowFlag[74] = 1; 
	PicShowFlag[85] = 1; 
	PicShowFlag[99] = 1; 
	/* ��ȡ Flash Device ID */
	DeviceID = SPI_FLASH_ReadDeviceID();

	/* ��ȡ SPI Flash ID */
	FlashID = SPI_FLASH_ReadID();
	printf("\r\nFlashID is 0x%X \r\nDevice ID is 0x%X\r\n", FlashID, DeviceID);

	/* ���� SPI Flash ID */
	if (FlashID == W25Q16ID || W25Q32ID || W25Q64ID || W25Q80ID)
	{
		printf("\r\n��⵽����flashоƬ!\r\n");
		switch (FlashID)
		{
		case W25Q16ID:
			printf("\r\nflashоƬ�ͺ�ΪW25Q16ID!\r\n");
			Flash_Size = 2;
			break;
		case W25Q32ID:
			printf("\r\nflashоƬ�ͺ�ΪW25Q32!\r\n");
			Flash_Size = 4;
			break;
		case W25Q64ID:
			printf("\r\nflashоƬ�ͺ�ΪW25Q64!\r\n");
			Flash_Size = 8;
			break;
		case W25Q80ID:
			printf("\r\nflashоƬ�ͺ�ΪW25Q80!\r\n");
			Flash_Size = 1;
			break;
		default:
			printf("\r\nflashоƬ�ͺ�Ϊ����!\r\n");
			Flash_Size = 0;
			break;
		}
		
		//FlashRW();//FLASH��д����
		
	} else {
		printf("\r\n��ȡ���� W25Q64 ID!\n\r");
	}

	delay_ms(500);
	LED = 1;//PC13Ϩ��
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
				//дFLASH
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
				ScreenMain(); //������ҳ��
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
				//�¶�
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

				//����ѹ
				LCD_ShowString(2 + 12 + 6 * 6,      65, " P:", BLUE, WHITE, 12, 0);
        LCD_ShowIntNumLeadingZeros(2 + 12 + 6 * 6 + 3 * 6, 65, (int)bmp280_press, 4, GRAYBLUE, WHITE, 12);
				LCD_ShowFloatNum1(         2 + 12 + 6 * 6 + 3 * 6 + 4 * 6, 65, (bmp280_press - (int)bmp280_press) * 100, 4, GRAYBLUE, WHITE, 12);
				LCD_ShowString(            2 + 12 + 6 * 6 + 3 * 6 + 4 * 6 + 5 * 6, 65, "Pa", BLUE, WHITE, 12, 0);
				//����
				LCD_ShowIntNumLeadingZeros(2 + 12 + 6 * 6 + 3 * 6 + 4 * 6 + 5 * 6 + 2 * 6, 65, high, 4, GRAYBLUE, WHITE, 12);
				//LCD_ShowFloatNum1(         2 + 12 + 6 * 6 + 3 * 6 + 4 * 6 + 5 * 6 + 2 * 6, 65, high / 100, 3, GRAYBLUE, WHITE, 12);

			//} else if (nSubMenu == 3) { //module info

			} else if(nSubMenu == 2) { //��BMP280��Ϣҳ��
	  		//�¶�
				//bmp280_temp -= 40;
  			LCD_ShowChinese(2, 12, "�¶ȣ�", LBBLUE, WHITE, 16, 0);
				//LCD_ShowString(12, 16, "TEMP:", BLUE, WHITE, 16, 0);
				if (bmp280_temp < 0) {
					LCD_ShowString(2 + 6 * 8, 6, "-", GRAYBLUE, WHITE, 24, 0);
					LCD_ShowFloatNum1(2 + 6 * 8 + 12, 6, -bmp280_temp, 4, GRAYBLUE, WHITE, 24);
					LCD_ShowChinese(2 + 6 * 8 + 6 * 12, 10, "��", GRAYBLUE, WHITE, 16, 0);			
				} else {
					LCD_ShowFloatNum1(2 + 6 * 8, 6, bmp280_temp, 4, GRAYBLUE, WHITE, 24);
					LCD_ShowChinese(2 + 6 * 8 + 5 * 12, 10, "��", GRAYBLUE, WHITE, 16, 0);
				}
				//����ѹ
				LCD_ShowChinese(0, 32, "����ѹ��", LBBLUE, WHITE, 16, 0);
				//LCD_ShowString(12, 32, "PRESS:", BLUE, WHITE, 16, 0);
				LCD_ShowIntNumLeadingZeros(0 + 8 * 8, 32, (int)bmp280_press, 4, GRAYBLUE, WHITE, 16);
				LCD_ShowString(0 + 8 * 8 + 4 * 8, 32, ".", BLUE, WHITE, 16, 0);
				LCD_ShowIntNumLeadingZeros(0 + 8 * 8 + 4 * 8 + 1 * 8, 32, (bmp280_press - (int)bmp280_press) * 10000, 4, GRAYBLUE, WHITE, 16);
				LCD_ShowString(0 + 8 * 8 + 4 * 8 + 1 * 8 + 4 * 8, 32, "hPa", GRAYBLUE, WHITE, 16, 0);

				//����
				LCD_ShowChinese(2, 52, "���Σ�", LBBLUE, WHITE, 16, 0);
				//LCD_ShowString(12, 48, "ASL:", BLUE, WHITE, 16, 0);
				LCD_ShowIntNum(2 + 6 * 8, 52, (int)high, 7, GRAYBLUE, WHITE, 16);
				//LCD_ShowString(2 + 6 * 8 + 7 * 8, 52, "CM", GRAYBLUE, WHITE, 16, 0);
			}
		}
		switch(g_KeyActionFlag)
    {
			case SHORT_KEY:
				//ִ�ж̰���Ӧ���¼�
			  printf("KEY: SINGLE CLICK.\r\n");
				if(bScreenMain == 0) { //����ҳ�淵��
					ScreenMain();
					bScreenMain = 1;
					nSubMenu = 0;
				} else { //����ҳ��
					if (bScreenSaver == 1) { //�������
						bScreenSaver = 0;
						ScreenMain();
					}
					nCountSingleClick++;
					nCountSingleClick = nCountSingleClick > 3 ? 0 : nCountSingleClick;

					LCD_Fill(0, 25 + 34, LCD_W, 25 + 36 + 6, WHITE); //���gImage_x��ָʾ^

					ScreenMainItem();
					//nCountSingleClick++;
				}
				g_KeyActionFlag = NULL_KEY; //(KEY_TYPE)0;		//״̬�ص���
				break;

			case LONG_KEY:
				//ִ�г�����Ӧ���¼�
			  printf("KEY: HOLD CLICK.\r\n");
				//LCD_ShowString(2, 60, "KEY PRESS LONG ", BLUE, WHITE, 16, 0);
				if (nCountSingleClick == 1 && bScreenMain == 1) { //BMP280 info
					bScreenMain = 0; //���Ϊ����ҳ��
					nSubMenu = 2;
					LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
				} else if (nCountSingleClick == 2 && bScreenMain == 1) { //module info
					bScreenMain = 0; //���Ϊ����ҳ��
					//nSubMenu = 3;
					LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
					LCD_ShowString(0,  0, "MCU: STM32F103C8T6", LBBLUE, WHITE, 16, 0);
					LCD_ShowString(0, 16, "IPS: 0.96' 80RGB*160", LBBLUE, WHITE, 16, 0);
					LCD_ShowString(0, 32, "LCD Driver: ST7735S", LBBLUE, WHITE, 16, 0);
					LCD_ShowString(0, 48, "Pressure Sen: BMP280", LBBLUE, WHITE, 16, 0);
					LCD_ShowString(0, 64, "Power SOC: IP5306", LBBLUE, WHITE, 16, 0);
				} else if (nCountSingleClick == 3 && bScreenMain == 1) { //����ҳ���"Կ��"��ť�ϳ���
					bScreenSaver = 1;
					LCD_Fill(0, 0, LCD_W, LCD_H, BLACK); //ȫ����ɫ���
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
						LCD_ShowString(9, 0, "Then, My Upload Pic", LBBLUE, WHITE, 16, 1);//���ӼӴ�
						LCD_ShowString(8, 16, "Width:160DOT", BLACK, WHITE, 16, 0);
						LCD_ShowString(8, 32, "High:80DOT", BLACK, WHITE, 16, 0);
						LCD_ShowString(8, 48, "Color:16bits(565)", BLACK, WHITE, 16, 0);
						LCD_ShowString(8, 64, "MSB First:Yes", BLACK, WHITE, 16, 0);
					} else {
						ShowPicFlash(nCountHoldClick);
					}
					nCountHoldClick++;

					bScreenMain = 0; //���Ϊ����ҳ��
				}
				g_KeyActionFlag = NULL_KEY; //(KEY_TYPE)0;		//״̬�ص���
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
		/* ������Ҫд��� SPI FLASH ������FLASHд��ǰҪ�Ȳ��� */
		// �������4K����һ����������������С��λ������
		SPI_FLASH_SectorErase(FLASH_SectorToErase);
		
		/* �����ͻ�����������д��flash�� */
		// ����дһҳ��һҳ�Ĵ�СΪ256���ֽ�
		SPI_FLASH_BufferWrite(Tx_Buffer, FLASH_WriteAddress, BufferSize);
		printf("\r\n д�������Ϊ��%s \r\t", Tx_Buffer);
		
		/* ���ո�д������ݶ������ŵ����ջ������� */
		SPI_FLASH_BufferRead(Rx_Buffer, FLASH_ReadAddress, BufferSize);
		printf("\r\n ����������Ϊ��%s \r\n", Rx_Buffer);
		
		/* ���д�������������������Ƿ���� */
		TransferStatus1 = Buffercmp(Tx_Buffer, Rx_Buffer, BufferSize);

		if (PASSED == TransferStatus1) {
			printf("\r\n ����flash���Գɹ�!\n\r");
		} else {
			printf("\r\n ����flash����ʧ��!\n\r");
		}
}

void ScreenMain()
{
	LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);

	LCD_ShowString(5, 0, "ZoomeXplore.cn", RED, WHITE, 16, 0); //LCD_ShowChinese(95+24, 0, "̽��", RED, WHITE, 16, 0);
	LCD_ShowPicture(160 - 16, 0, 16, 16, gImage_logo16x16); //logo
	//LCD_ShowString(10, 20, "LCD_W:", RED, WHITE, 16, 0);
	//LCD_ShowIntNum(58, 20, LCD_W, 3, RED, WHITE, 16);
	//LCD_ShowString(10, 40, "LCD_H:", RED, WHITE, 16, 0);
	//LCD_ShowIntNum(58, 40, LCD_H, 3, RED, WHITE, 16);
	LCD_ShowPicture(140, 25, 16, 32, gImage_4); //Կ��ͼ�ꡢ����
	LCD_ShowPicture(95, 18, 36, 40, gImage_3);  //ϵͳ��Ϣ
	LCD_ShowPicture(45, 30, 40, 28, gImage_2);  //BMP-280�¶ȡ�����ѹ����
	LCD_ShowPicture(5,  20, 33, 38, gImage_1);
	
	ScreenMainItem();
  //LCD_ShowString(2, 65, "SINGLE/HOLD CLICK", BLUE, WHITE, 12, 0);
	//LCD_ShowString(115, 65, "ROM:", GRAYBLUE, WHITE, 12, 0); 
	LCD_ShowIntNum(73+70, 65, Flash_Size, 1, GRAYBLUE, WHITE, 12); LCD_ShowString(79+70, 65, "M", GRAYBLUE, WHITE, 12, 0);
}

void ScreenMainItem(void)
{
	LCD_ShowString(20 + nCountSingleClick * 40 + 3, 25 + 33, "^", RED, WHITE, 16, 1);//gImage_x��ָʾ
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
	//�⺯���汾
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
	
	//�Ĵ����汾
	//----------------------------------------------------------------//
//	RCC->APB1ENR |= (uint32_t)0x00000002; //�򿪶�ʱ��3��ʱ��
//	TIM3->CR1 &= (uint16_t)0xFF8F; //CMS[1:0]��DIRλ����
//	TIM3->CR1 |= (uint16_t)0x0000;   //���ü���Ϊ���ϼ���
//	
//	TIM3->CR1 &= (uint16_t)0xFCFF; //CKD[1:0]λ����
//	TIM3->CR1 |= (uint16_t)0x0000;   //����1��Ƶ
//	
//	TIM3->ARR =  9999; //�����Զ�װ��ֵ
//	TIM3->PSC =  7199; //����Ԥ��Ƶֵ
//	
//	TIM3->EGR = (uint16_t)0x0001;
//	
//	TIM3->DIER |= (uint16_t)0x0001;
//	
//  my_tmppriority = (0x700 - ((SCB->AIRCR) & (uint32_t)0x700))>> 0x08;
//  my_tmppre = (0x4 - my_tmppriority);
//  my_tmpsub = my_tmpsub >> my_tmppriority;

//  my_tmppriority = 1 << my_tmppre; //��ռ���ȼ�Ϊ1
//  my_tmppriority |=  2 & my_tmpsub;//�����ȼ�Ϊ2
//  my_tmppriority = my_tmppriority << 0x04;
//	
//	NVIC->IP[TIM3_IRQn] = my_tmppriority;
//	NVIC->ISER[TIM3_IRQn >> 0x05] =(uint32_t)0x01 << (TIM3_IRQn & (uint8_t)0x1F);//����ͨ��
//	//NVIC->ISER[0] =1<<29;
//	TIM3->CR1 |= (uint16_t)0x0001;
}

/**
 * �����������̰��ͳ����¼�
 * �̰���ʱ�� 10ms < T < 1 s, ������ʱ�� T >1 s
 * ���ܣ�ʹ��״̬����ʽ��ɨ�赥��������ɨ������Ϊ10ms,10ms�պ�����������
 * ״̬��ʹ��switch case���ʵ��״̬֮�����ת
 * lock���������ж��Ƿ��ǵ�һ�ν��а���ȷ��״̬
 * �������ͷź��ִ�а����¼�
 */
void Key_Scan0(void)
{
    static u8 TimeCnt = 0;
    static u8 lock = 0;
    switch(KeyState)
    {
        //����δ����״̬����ʱ�ж�Key��ֵ
        case KEY_CHECK:    
					if(Key)   //!Key
					{
						KeyState =  KEY_COMFIRM;  //�������KeyֵΪ0��˵��������ʼ���£�������һ��״̬ 
					}
					TimeCnt = 0;                  //������λ
					lock = 0;
					break;
            
        case KEY_COMFIRM:
					if(Key)  //!Key                   //�鿴��ǰKey�Ƿ���0���ٴ�ȷ���Ƿ���
					{
						if(!lock)   lock = 1;
						TimeCnt++;
					}   
					else                       
					{
							if(lock)                // ���ǵ�һ�ν��룬  �ͷŰ�����ִ��
							{
									//����ʱ���ж�
									if(TimeCnt > 100)            // ���� 1 s
									{
											g_KeyActionFlag = LONG_KEY;
											TimeCnt = 0;  
									}
									else                         // Keyֵ��Ϊ��1��˵���˴�����Ϊ�̰�
									{
											g_KeyActionFlag = SHORT_KEY;          // �̰�
									}
									
									KeyState =  KEY_RELEASE;    // ��Ҫ���밴���ͷ�״̬ 
							}
							else                          // ��ǰKeyֵΪ1��ȷ��Ϊ�������򷵻���һ��״̬
							{
									KeyState =  KEY_CHECK;    // ������һ��״̬
							}
					} 
          break;
            
        case KEY_RELEASE:
          if(!Key) //Key                    //��ǰKeyֵΪ1��˵�������Ѿ��ͷţ����ؿ�ʼ״̬
          { 
            KeyState =  KEY_CHECK;    
          }
          break;
             
        default: 
					break;
    }    
}

/**
 * �����������̰��ͳ����¼�
 * �̰���ʱ�� 10ms < T < 1 s, ������ʱ�� T >1 s
 * ���ܣ�ʹ��״̬����ʽ��ɨ�赥��������ɨ������Ϊ10ms,10ms�պ�����������
 * ״̬��ʹ��switch case���ʵ��״̬֮�����ת
 * lock���������ж��Ƿ��ǵ�һ�ν��а���ȷ��״̬
 * �������¼���ǰִ�У��̰����¼��ͷź��ִ��
 */
//u8 TimeCnt = 0;
//u8 lock = 0;
void Key_Scan(void)
{
		static u8 TimeCnt = 0;
    static u8 lock = 0;
	
    switch (KeyState)
    {
        //����δ����״̬����ʱ�ж�Key��ֵ
        case KEY_CHECK:    
           if(Key)   //!Key 
            {
							//printf("KeyState=KEY_CHECK !Key\r\n");
              KeyState = KEY_COMFIRM;  //�������KeyֵΪ0��˵��������ʼ���£�������һ��״̬ 
            }
            TimeCnt = 0;                  //������λ
            lock = 0;
            break;
            
        case KEY_COMFIRM:
            if(Key)  //!Key                   //�鿴��ǰKey�Ƿ���0���ٴ�ȷ���Ƿ���
            {
              if(!lock) lock = 1;
               	
							//printf("KeyState=KEY_COMFIRM !Key\r\n");

              TimeCnt++;  
                
              //����ʱ���ж�
              if(TimeCnt > 100)            // ���� 1 s
              {
                g_KeyActionFlag = LONG_KEY;
                TimeCnt = 0;  
                lock = 0;               //���¼��
                KeyState = KEY_RELEASE;    // ��Ҫ���밴���ͷ�״̬
              }
            }   
            else                       
            {
              if(1 == lock)                // ���ǵ�һ�ν��룬  �ͷŰ�����ִ��
              {
                g_KeyActionFlag = SHORT_KEY;          // �̰�
                KeyState = KEY_RELEASE;    // ��Ҫ���밴���ͷ�״̬ 
              }
              else                          // ��ǰKeyֵΪ1��ȷ��Ϊ�������򷵻���һ��״̬
              {
                KeyState = KEY_CHECK;    // ������һ��״̬
              }
            } 
            break;
            
         case KEY_RELEASE:
             if(!Key)   //Key                  //��ǰKeyֵΪ1��˵�������Ѿ��ͷţ����ؿ�ʼ״̬
             { 
                 KeyState = KEY_CHECK;    
             }
						 break;
             
         default: 
					 break;
    }    
}

void TIM3_IRQHandler(void)   //TIM3 ÿ 2ms �ж�һ��
{   
	static u8 cnt;
	static u16 cnt_bmp280; //	1000msȡһ��BMP280����

	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ    
	{
		cnt++;
		cnt_bmp280++;
		
    if(cnt > 5)	//ÿ10ms ִ��һ�ΰ���ɨ�����
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
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx���жϴ�����λ:TIM �ж�Դ
	}
}

//ͼƬ����д��Flash��������š�ͷ��Ϣ������ÿ��д�������ص����ݣ�
//nPkgNoΪ0ʱΪ����ͷ��    ����Э��ͷ"ZXI_HEAD"(8BYTE)+ͼ����(1BYTE)+�����0(1BYTE)+���MSB(1BYTE)+���LSB(1BYTE)+�߶�MSB(1BYTE)+�߶�LSB(1BYTE)��Ϣ
//nPkgNo>0ʱΪͼ��RGB���ݣ�����Э��ͷ"ZXI_DATA"(8BYTE)+ͼ����(1BYTE)+�����N(1BYTE)+RGB����(n BYTE)��ÿ�δ�200�ֽ�ͼ����Ч��Ϣ�����һ������200��0x00���
int Pic2Flash(int nNo, int nPkgNo)
{
	u32 nAdrStart = 0x00000;
	
	nAdrStart = nAdrStart + (nNo - 1) * 4 * 1024 * 8; //ÿ����ŵ����ݷ���160*80*2=25600<4*1024*8=32768�ֽڵĿռ�
	//return 1;

	//������Ҫд��� SPI FLASH ������FLASHд��ǰҪ�Ȳ���
	//�������4K����һ����������������С��λ������,һ��ͼƬռ��32K�ռ�
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
	//�����ͻ�����������д��flash��
	//����дһҳ��һҳ�Ĵ�СΪ256���ֽ�,ʵ��д��208+2�ֽ�,W25Q�����ʴ�Լ78%
	nAdrStart += nPkgNo * 256;
	nBufferSize = (nPkgNo == 0) ? 14 : PKGSIZE_DATA;
	SPI_FLASH_BufferWrite(Tx_Buffer, nAdrStart, nBufferSize);
	
	if (nPkgNo == 1) {
	printf("ZXI>д�������Ϊ(%d): \r\n", nBufferSize);
	for (int i = 0; i < nBufferSize; i++) {
		printf("%02X ", Tx_Buffer[i]);
	}

	printf("\r\n------------\r\n");
	//���ո�д������ݶ������ŵ����ջ�������
	SPI_FLASH_BufferRead(Rx_Buffer, nAdrStart, nBufferSize);
	printf("ZXI>����������Ϊ(%d) \r\n", nBufferSize);
	for (int i = 0; i < nBufferSize; i++) {
		printf("%02X ", Rx_Buffer[i]);
	}
	printf("\r\n------------\r\n");
	//���д�������������������Ƿ����
	TransferStatus1 = Buffercmp(Tx_Buffer, Rx_Buffer, nBufferSize);

	if (PASSED == TransferStatus1) {
		printf("ZXI>Addr:0x%08X,����flashд���ɹ�!\r\n", nAdrStart);
		return 0; //д��ɹ�����0������PASSED
	} else {
		printf("ZXI>����flashд��ʧ��!\r\n");
		return 1; //д��ʧ�ܷ���1������FAILED
	}
	
	return 0;} 
	
	return 0;
}

//��ʾָ����ŵ�FLASH�ϵ�ͼƬ��TFT��
void ShowPicFlash(int nNo)
{
	//LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
	LCD_Address_Set(0, 0, 159, 79);
	for (int r = 1; r < 129; r++) {
		ShowPicLine(nCountHoldClick, r);
	}
	LCD_ShowIntNumLeadingZeros(8, 0, nCountHoldClick - 2, 3, GRAYBLUE, WHITE, 16);
}

//��ʾһ��ͼ������
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
//��ʾָ����ŵĴ��ڴ������ϵ�ͼƬ��TFT��
void ShowPicComm(int nNo, int nPkgNo)
{
	//int nNo = 3;
}
