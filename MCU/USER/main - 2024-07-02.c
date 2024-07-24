//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ABrobot����
//
//
//  �� �� ��   : main.c
//  �� �� ��   : v2.0
//  ��    ��   : ABrobot
//  ��������   : 2023-10-25
//  ����޸�   : 
//  ��������   :��ʾ����(STM32F103 TFT������ϵ�� ����������)
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
//              
//              ----------------------------------------------------------------
//
//							FLASHоƬ����
//              ----------------------------------------------------------------
//
//              GND   ��Դ��
//              VCC   3.3v��Դ
//              CS    PB12
//              CLK   PB13
//              DO    PB14
//              DI    PB15
//             
//              
//              ----------------------------------------------------------------
//��Ȩ���У�����ؾ���
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

//�����������
#ifdef SingleKeyEvent

//������Ӧ��IO�ܽ� KEY1  PA.15
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

KEY_STATE KeyState = KEY_CHECK;	// ��ʼ������״̬Ϊ���״̬
u8 g_KeyFlag = 1;								// ������Ч��־��0�� ����ֵ��Ч�� 1������ֵ��Ч

/**
 * ������������¼�
 * ���ܣ�ʹ��״̬����ʽ��ɨ�赥��������ɨ������Ϊ10ms,10ms�պ�����������
 * ״̬��ʹ��switch case���ʵ��״̬֮�����ת
 * 
 */
void Key_Scan(void)
{
		switch (KeyState)
    {
        //����δ����״̬����ʱ�ж�Key��ֵ
        case	KEY_CHECK:    
            if(!Key)   
            {
                KeyState =  KEY_COMFIRM;	//�������KeyֵΪ0��˵��������ʼ���£�������һ��״̬
            }
            break;
				//��������״̬��
        case	KEY_COMFIRM:
            if(!Key)											//�鿴��ǰKey�Ƿ���0���ٴ�ȷ���Ƿ���
            {
                KeyState =  KEY_RELEASE;	//������һ���ͷ�״̬
                g_KeyFlag = 1;						//������ЧֵΪ1, ����ȷ�ϰ��£����¾�ִ�а�������        
            }   
            else													//��ǰKeyֵΪ1��ȷ��Ϊ�������򷵻���һ��״̬
            {
                KeyState =  KEY_CHECK;		//������һ��״̬
            } 
            break;
				//�����ͷ�״̬
        case	KEY_RELEASE:
             if(Key)                     //��ǰKeyֵΪ1��˵�������Ѿ��ͷţ����ؿ�ʼ״̬
             { 
                 KeyState =  KEY_CHECK;
               //  g_KeyFlag = 1;        //������ڴˣ����ڰ����ͷ�״̬�󣬲�ִ�а�������
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

/* ��ȡ�������ĳ��� */
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

// ����ԭ������
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);

int main(void)
{
	float t = 0;
	__IO uint32_t Flash_Size = 0;
	delay_init();
	LED_Init();//LED��ʼ��
	Usart1_Init(115200);
	SPI_FLASH_Init();

	LCD_Init();//LCD��ʼ��
	//LCD_BLK_Set();//�򿪱���
	LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
	
	LED = 1;


	/* ��ȡ Flash Device ID */
	DeviceID = SPI_FLASH_ReadDeviceID();

	/* ��ȡ SPI Flash ID */
	FlashID = SPI_FLASH_ReadID();
	printf("\r\n FlashID is 0x%X \r\n Device ID is 0x%X\r\n", FlashID, DeviceID);


	/* ���� SPI Flash ID */
	if (FlashID == W25Q16ID || W25Q32ID || W25Q64ID || W25Q80ID)
	{

		printf("\r\n ��⵽����flashоƬ!\r\n");
		switch (FlashID)
		{
		case W25Q16ID:
			printf("\r\n flashоƬ�ͺ�ΪW25Q16ID!\r\n");
			Flash_Size = 2;
			break;
		case W25Q32ID:
			printf("\r\n flashоƬ�ͺ�ΪW25Q32!\r\n");
			Flash_Size = 4;
			break;
		case W25Q64ID:
			printf("\r\n flashоƬ�ͺ�ΪW25Q64!\r\n");
			Flash_Size = 8;
			break;
		case W25Q80ID:
			printf("\r\n flashоƬ�ͺ�ΪW25Q80!\r\n");
			Flash_Size = 1;
			break;
		default:
			printf("\r\n flashоƬ�ͺ�Ϊ����!\r\n");
			Flash_Size = 0;
			break;
		}

		/* ������Ҫд��� SPI FLASH ������FLASHд��ǰҪ�Ȳ��� 
		// �������4K����һ����������������С��λ������
		//SPI_FLASH_SectorErase(FLASH_SectorToErase);
		*/
		/* �����ͻ�����������д��flash�� 
		// ����дһҳ��һҳ�Ĵ�СΪ256���ֽ�
		SPI_FLASH_BufferWrite(Tx_Buffer, FLASH_WriteAddress, BufferSize);
		printf("\r\n д�������Ϊ��%s \r\t", Tx_Buffer);
		*/

		/* ���ո�д������ݶ������ŵ����ջ������� 
		SPI_FLASH_BufferRead(Rx_Buffer, FLASH_ReadAddress, BufferSize);
		printf("\r\n ����������Ϊ��%s \r\n", Rx_Buffer);
		*/

		/* ���д�������������������Ƿ���� 
		TransferStatus1 = Buffercmp(Tx_Buffer, Rx_Buffer, BufferSize);

		if (PASSED == TransferStatus1)
		{

			printf("\r\n ����flash���Գɹ�!\n\r");
		}
		else
		{

			printf("\r\n ����flash����ʧ��!\n\r");
		}
		*/
	}// if (FlashID == sFLASH_ID)
	else// if (FlashID == sFLASH_ID)
	{

		printf("\r\n ��ȡ���� W25Q64 ID!\n\r");
	}



		delay_ms(500);
		//LED = 1;//PC13Ϩ��	
		delay_ms(500);
	
	while (1)
	{
		Key_Scan();
		delay_ms(10);
		
		if (g_KeyFlag == 1) {
		//LED = 0; //PC13����
		//��Ļ������ʾ	
		LCD_ShowString(5, 0, "ZoomeXplore", RED, WHITE, 16, 0); LCD_ShowChinese(95, 0, "����̽��", RED, WHITE, 16, 0);
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
