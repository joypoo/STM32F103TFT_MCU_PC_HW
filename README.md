HW
参与贡献
STM32F103TFT_MCU_PC_HW
介绍 基于STM32F103的便携式存储棒

功能：

​ 温度、大气压、海拔采集及展示；

​ USB-C充电、小型充电宝；

​ 上位机传输图片到存储棒IPS TFT屏显示功能。

下位机：

​ 基于M3 ARM核的STM32F103C8T6+IPS160*80DOT点阵屏+W25Q32+充放电模块

上位机：

​ 基于VS2022(VC++)+MFC创建的桌面程序

软件架构

终端为MCU串口通信程序，PC端为单机版程序

安装教程

通过Type-C数据线连接Windows主机

使用说明

1、主机操作 1.1 在主页面：长按【Home】键进入相应子页面。 1.2 在子页面：短按【Home】键返回主页面，在[图像浏览]页面长按【Home】键可进行下一图片预览。 1.3 操作进入息屏后，短按【Home】键可返回正常显示模式。 1.4 Type-C通信口用于与上位机软件通信，亦可作供电电源，供电电压为5.0V。 1.5 所有源代码可随意更改、编译，开发环境为Keil V5.36 Toolchain为MDK-ARM，程序可通过Type-C通信口使用mcuisp软件或SWD口使用Keil开发环境或Segger J-Flash烧写。

2、上位机程序联机操作 2.1 通过Type-C数据线连接Windows主机（建议Windows10及以上x64系统）。 2.2 PC机需安装有CH340驱动程序，驱动可到官网安装下载。 2.3 导入的图片建议用16080DOT或成比例放大的.jpg/.bmp格式，由Image2Lcd软件生成16080DOT的16位色带有图像头数据的bin文件，扫描模式为水平扫描。 2.4 上位机软件中的图像编号为保存到下位机Flash中的图像编号。 2.5 所有源代码可随意更改，开发环境为VS2022（VC++）。
1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
