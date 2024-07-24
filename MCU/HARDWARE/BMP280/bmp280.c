/*
 BMP280的测试启动流程
（1）读取BMP280的id，其值等于0x58。
（2）把补偿寄存器的值都读出来。
（3）对BMP280进行reset。
（4）对BMP280的数据采集精度，模式。。。等进行配置。
（5）给点延时，等待数据采集完毕放入寄存器中。
（6）读取寄存器中的采集数据。
 */
 
#include "bmp280.h"
#include <stdio.h>

//初始化IIC
void BMP_IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	//RCC->APB2ENR|=1<<4;//先使能外设IO PORTC时钟 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11; //GPIO_Pin_12|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;                            //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure); //(GPIOC,&GPIO_InitStructure);
 
	IIC_SCL=1;
	IIC_SDA=1;

}
//产生IIC起始信号
void BMP_IIC_Start(void)
{
  SDA_OUT();                                                                 //sda线输出
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;                                                                 //START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL=0;                                                                 //钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void BMP_IIC_Stop(void)
{
	SDA_OUT();                                                                 //sda线输出
  IIC_SCL=0;
	IIC_SDA=0;                                                                 //STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;                                                                 //发送I2C总线结束信号
	delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 BMP_IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();                                                                  //SDA设置为输入  
	IIC_SDA=1;delay_us(1);	   
	IIC_SCL=1;delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			BMP_IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;                                                                 //时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void BMP_IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}
//不产生ACK应答		    
void BMP_IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void BMP_IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
		SDA_OUT(); 	    
    IIC_SCL=0;                                                                 //拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
				delay_us(2);                                                           //对TEA5767这三个延时都是必须的
				IIC_SCL=1;
				delay_us(2); 
				IIC_SCL=0;	
				delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 BMP_IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();                                                                  //SDA设置为输入
  for(i=0;i<8;i++ )
	{
    IIC_SCL=0; 
    delay_us(2);
		IIC_SCL=1;
    receive<<=1;
    if(READ_SDA)receive++;   
		delay_us(1); 
  }					 
  if (!ack)
    BMP_IIC_NAck();                                                            //发送nACK
  else
    BMP_IIC_Ack();                                                             //发送ACK   
  return receive;
}

//从指定地址读出一个数据
//ReadAddr:开始读数的地址  
//返回值  :读到的数据
u8 BMP_iicDevReadByte(u8 devaddr,u8 addr)
{				  
	u8 temp=0;		  	    																 
	BMP_IIC_Start();  
	BMP_IIC_Send_Byte(devaddr);                                                    //发送器件写命令 	   
	BMP_IIC_Wait_Ack(); 
	BMP_IIC_Send_Byte(addr);                                                       //发送低地址
	BMP_IIC_Wait_Ack();	

	BMP_IIC_Start();  	 	   
	BMP_IIC_Send_Byte(devaddr|1);                                                  //发送器件读命令			   
	BMP_IIC_Wait_Ack();	 
	temp=BMP_IIC_Read_Byte(0);			   
	BMP_IIC_Stop();                                                                //产生一个停止条件	    
	return temp;
}

//连续读多个字节
//addr:起始地址
//rbuf:读数据缓存
//len:数据长度
void BMP_iicDevRead(u8 devaddr,u8 addr,u8 len,u8 *rbuf)
{
	int i=0;
	BMP_IIC_Start();  
	BMP_IIC_Send_Byte(devaddr);  
	BMP_IIC_Wait_Ack();	
	BMP_IIC_Send_Byte(addr);                                                       //地址自增  
	BMP_IIC_Wait_Ack();	

	BMP_IIC_Start();  	
	BMP_IIC_Send_Byte(devaddr|1);  	
	BMP_IIC_Wait_Ack();	
	for(i=0; i<len; i++)
	{
		if(i==len-1)
		{
			rbuf[i]=BMP_IIC_Read_Byte(0);                                          //最后一个字节不应答
		}
		else
			rbuf[i]=BMP_IIC_Read_Byte(1);
	}
	BMP_IIC_Stop( );	
}

//从指定地址写入一个数据
//WriteAddr :写入数据的目的地址    
//DataToWrite:要写入的数据
void BMP_iicDevWriteByte(u8 devaddr,u8 addr,u8 data)
{				   	  	    																 
	BMP_IIC_Start();  
	BMP_IIC_Send_Byte(devaddr);                                                    //发送器件写命令 	 
	BMP_IIC_Wait_Ack();	   
	BMP_IIC_Send_Byte(addr);                                                       //发送低地址
	BMP_IIC_Wait_Ack(); 	 										  		   
	BMP_IIC_Send_Byte(data);                                                       //发送字节							   
	BMP_IIC_Wait_Ack();  		    	   
	BMP_IIC_Stop();		                                                           //产生一个停止条件 	 
}

//连续写多个字节
//addr:起始地址
//wbuf:写数据缓存
//len:数据的长度
void BMP_iicDevWrite(u8 devaddr,u8 addr,u8 len,u8 *wbuf)
{
	int i=0;
	BMP_IIC_Start();  
	BMP_IIC_Send_Byte(devaddr);  	
	BMP_IIC_Wait_Ack();	
	BMP_IIC_Send_Byte(addr);  //地址自增
	BMP_IIC_Wait_Ack();	
	for(i=0; i<len; i++)
	{
		BMP_IIC_Send_Byte(wbuf[i]);  
		BMP_IIC_Wait_Ack();		
	}
	BMP_IIC_Stop( );	
}



/*bmp280 气压和温度过采样 工作模式*/
#define BMP280_PRESSURE_OSR			(BMP280_OVERSAMP_8X)
#define BMP280_TEMPERATURE_OSR		(BMP280_OVERSAMP_16X)
#define BMP280_MODE					(BMP280_PRESSURE_OSR<<2|BMP280_TEMPERATURE_OSR<<5|BMP280_NORMAL_MODE)


typedef struct 
{
    u16 dig_T1;                                             /* calibration T1 data */
    s16 dig_T2;                                             /* calibration T2 data */
		s16 dig_T3;                                             /* calibration T3 data */
    u16 dig_P1;                                             /* calibration P1 data */
    s16 dig_P2;                                             /* calibration P2 data */
    s16 dig_P3;                                             /* calibration P3 data */
    s16 dig_P4;                                             /* calibration P4 data */
    s16 dig_P5;                                             /* calibration P5 data */
    s16 dig_P6;                                             /* calibration P6 data */
    s16 dig_P7;                                             /* calibration P7 data */
    s16 dig_P8;                                             /* calibration P8 data */
    s16 dig_P9;                                             /* calibration P9 data */
    s32 t_fine;                                             /* calibration t_fine data */
} bmp280Calib;

bmp280Calib  bmp280Cal;

static u8 bmp280ID=0;
static bool isInit=false;
static s32 bmp280RawPressure=0;
static s32 bmp280RawTemperature=0;

static void bmp280GetPressure(void);
static void presssureFilter(float* in,float* out);
static float bmp280PressureToAltitude(float* pressure/*, float* groundPressure, float* groundTemp*/);

bool bmp280Init(void)
{	
  if (isInit)
    return true;

	BMP_IIC_Init();		                                                           /*初始化I2C*/
  delay_ms(20);
	
	bmp280ID=BMP_iicDevReadByte(BMP280_ADDR,BMP280_CHIP_ID);	                   /* 读取bmp280 ID*/
	
	if(bmp280ID==BMP280_DEFAULT_CHIP_ID) {
		printf("BMP280 ID IS: 0x%X\r\n",bmp280ID);
	}
  else {
		printf("BMP280 ID ERR IS: 0x%X\n",bmp280ID);

    return false;
	}

  // 读取校准数据
  BMP_iicDevRead(BMP280_ADDR, BMP280_TEMPERATURE_CALIB_DIG_T1_LSB_REG, 24, (u8 *)&bmp280Cal);
	
	BMP_iicDevWriteByte(BMP280_ADDR, BMP280_CTRL_MEAS_REG, BMP280_MODE);
	BMP_iicDevWriteByte(BMP280_ADDR, BMP280_CONFIG_REG, 5 << 2);		               /*配置IIR滤波*/
	
//	printf("BMP280 Calibrate Registor Are dig_T1: 0x%04X\r\n", 0xFFFF & (bmp280Cal.dig_T1));
//	printf("BMP280 Calibrate Registor Are dig_T2: 0x%04X\r\n", 0xFFFF & (bmp280Cal.dig_T2));
//	printf("BMP280 Calibrate Registor Are dig_T3: 0x%04X\r\n", 0xFFFF & (bmp280Cal.dig_T3));
//	printf("BMP280 Calibrate Registor Are dig_P1: 0x%04X\r\n", 0xFFFF & (bmp280Cal.dig_P1));
//	printf("BMP280 Calibrate Registor Are dig_P2: 0x%04X\r\n", 0xFFFF & (bmp280Cal.dig_P2));
//	printf("BMP280 Calibrate Registor Are dig_P3: 0x%04X\r\n", 0xFFFF & (bmp280Cal.dig_P3));
//	printf("BMP280 Calibrate Registor Are dig_P4: 0x%04X\r\n", 0xFFFF & (bmp280Cal.dig_P4));
//	printf("BMP280 Calibrate Registor Are dig_P5: 0x%04X\r\n", 0xFFFF & (bmp280Cal.dig_P5));
//	printf("BMP280 Calibrate Registor Are dig_P6: 0x%04X\r\n", 0xFFFF & (bmp280Cal.dig_P6));
//	printf("BMP280 Calibrate Registor Are dig_P7: 0x%04X\r\n", 0xFFFF & (bmp280Cal.dig_P7));
//	printf("BMP280 Calibrate Registor Are dig_P8: 0x%04X\r\n", 0xFFFF & (bmp280Cal.dig_P8));
//	printf("BMP280 Calibrate Registor Are dig_P9: 0x%04X\r\n", 0xFFFF & (bmp280Cal.dig_P9));
	
//	bmp280Calib *p = &bmp280Cal;
//	
//	for(int i = 0; i < 24; i++)
//		printf("Registor %2d: 0x%X ", i, p[i]);
//	printf("\r\n");
	
  isInit = true;
  return true;
}

static void bmp280GetPressure(void)
{
    u8 data[BMP280_DATA_FRAME_SIZE];

    // read data from sensor
    BMP_iicDevRead(BMP280_ADDR,BMP280_PRESSURE_MSB_REG,BMP280_DATA_FRAME_SIZE,data);
    bmp280RawPressure=(s32)((((uint32_t)(data[0]))<<12)|(((uint32_t)(data[1]))<<4)|((uint32_t)data[2]>>4));
    bmp280RawTemperature=(s32)((((uint32_t)(data[3]))<<12)|(((uint32_t)(data[4]))<<4)|((uint32_t)data[5]>>4));
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of "5123" equals 51.23 DegC
// t_fine carries fine temperature as global value
static s32 bmp280CompensateT(s32 adcT)
{
    s32 var1, var2, T;

    var1=((((adcT >> 3) - ((s32)bmp280Cal.dig_T1 << 1))) * ((s32)bmp280Cal.dig_T2)) >> 11;
    var2=(((((adcT >> 4) - ((s32)bmp280Cal.dig_T1)) * ((adcT >> 4) - ((s32)bmp280Cal.dig_T1))) >> 12) * ((s32)bmp280Cal.dig_T3)) >> 14;
    bmp280Cal.t_fine = var1 + var2;
	
    T = (bmp280Cal.t_fine * 5 + 128) >> 8;
		
    return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of "24674867" represents 24674867/256 = 96386.2 Pa = 963.862 hPa
static uint32_t bmp280CompensateP(s32 adcP)
{
    int64_t var1,var2,p;
    var1=((int64_t)bmp280Cal.t_fine)-128000;
    var2=var1*var1*(int64_t)bmp280Cal.dig_P6;
    var2=var2+((var1*(int64_t)bmp280Cal.dig_P5)<<17);
    var2=var2+(((int64_t)bmp280Cal.dig_P4)<<35);
    var1=((var1*var1*(int64_t)bmp280Cal.dig_P3)>>8)+((var1*(int64_t)bmp280Cal.dig_P2)<<12);
    var1=(((((int64_t)1)<<47)+var1))*((int64_t)bmp280Cal.dig_P1)>>33;
    if (var1==0)
        return 0;
    p=1048576-adcP;
    p=(((p<<31)-var2)*3125)/var1;
    var1=(((int64_t)bmp280Cal.dig_P9)*(p>>13)*(p>>13))>>25;
    var2=(((int64_t)bmp280Cal.dig_P8)*p)>>19;
    p=((p+var1+var2)>>8)+(((int64_t)bmp280Cal.dig_P7)<<4);
    return(uint32_t)p;
}

#define FILTER_NUM	5
#define FILTER_A	0.1f

/*限幅平均滤波法*/
static void presssureFilter(float* in,float* out)
{	
	static u8 i=0;
	static float filter_buf[FILTER_NUM]={0.0};
	double filter_sum=0.0;
	u8 cnt=0;	
	float deta;

	if(filter_buf[i]==0.0f)
	{
		filter_buf[i]=*in;
		*out=*in;
		if(++i>=FILTER_NUM)	
			i=0;
	} 
	else 
	{
		if(i)
			deta=*in-filter_buf[i-1];
		else 
			deta=*in-filter_buf[FILTER_NUM-1];
		
		if(fabs(deta)<FILTER_A)
		{
			filter_buf[i]=*in;
			if(++i>=FILTER_NUM)	
				i=0;
		}
		for(cnt=0;cnt<FILTER_NUM;cnt++)
		{
			filter_sum+=filter_buf[cnt];
		}
		*out=filter_sum /FILTER_NUM;
	}
}

void bmp280GetData(float* pressure,float* temperature,float* asl)
{
  static float t;
  static float p;
	
	bmp280GetPressure();

	t=bmp280CompensateT(bmp280RawTemperature)/100.0;
	
	t -= 2.5; //2024-7-11,实际比测量低3度，在这里暂时矫正。
	p=bmp280CompensateP(bmp280RawPressure)/25600.0;

	presssureFilter(&p,pressure);
	*temperature=(float)t;                                                     /*单位度*/
//	*pressure=(float)p ;	                                                   /*单位hPa*/	
	
	*asl=bmp280PressureToAltitude(pressure);	                               /*转换成海拔*/	
}

#define CONST_PF 0.1902630958	                                               //(1/5.25588f) Pressure factor
#define FIX_TEMP 25				                                               // Fixed Temperature. ASL is a function of pressure and temperature, but as the temperature changes so much (blow a little towards the flie and watch it drop 5 degrees) it corrupts the ASL estimates.
								                                               // TLDR: Adjusting for temp changes does more harm than good.
/*
 * Converts pressure to altitude above sea level (ASL) in meters
*/
static float bmp280PressureToAltitude(float* pressure/*, float* groundPressure, float* groundTemp*/)
{
//	float pressure_sea_level = 101325; // 标准海平面大气压力  
//  float altitude = 44330 * (1 - pow((*pressure/pressure_sea_level), 1 / 5.255));
//  
//	return altitude;
	
    if (*pressure>0)
    {
        return((pow((1015.7f/ *pressure),CONST_PF)-1.0f)*(FIX_TEMP+273.15f))/0.0065f;
    }
    else
    {
        return 0;
    }
}

