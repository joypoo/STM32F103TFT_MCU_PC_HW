// CommMFC.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号

#include <mysql.h>

using namespace std;

//传感器节点类
class CSensorNode {
public:
	int id;					//自动ID
	CString node_id;		//节点ID
	float	temp;			//节点温度
	float	tempc;			//电池温度
	CString	db_datetime;	//入库时间
	CString node_datetime;	//节点数据生成时间
};

//数据读写方式
enum TransType
{
	TRANSTYOE_CURL = 0,
	TRANSTYOE_MYSQL = 1,
	TRANSTYOE_MQTT = 2,
	TRANSTYOE_REDIS = 3
};

class CZXiGateApp : public CWinApp
{
public:
	CZXiGateApp();

// 重写
	public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()

public:
	CString strRetCurl;
	//static size_t write_memory_callback(void* contents, size_t size, size_t nmemb, void* userp);
	//int RunCurl();

	uint64_t node_rowcount; //记录总数
	void RunConnectMySQL(CString strSQL); //连接数据库、数据入库，函数暂停使用
	MYSQL mysql;
	int ConnectDatabase(void);		//连接数据库
	void DisConnectDatabase(void);	//断开数据库

	int DataLoading(CString strSQL); //数据写入数据库

	CSensorNode* sensor_node = new CSensorNode[1024];
	CString strHostDatebase; //数据库地址
	CString strUserDatabase; //数据库登录名
	CString strPasswdDatabase; //数据库登录密码
	CString strDbDatabase; //数据库名
	//MYSQL strErrDataase; //连接数据库返回的错误码

	int nTypeTranslateData; //DTU到平台的数据传输方式

	WORD wCamCmd; //相机的命令帧：0x5A23(Z#)-获取版本, 0x4823(H#)-拍摄照片,   0x4523(E#)-获取指定包数据, 
								//0x4423(D#)-更改ID,   0x4923(I#)-设置波特率, 0x5123(Q#)-设置压缩率。
	WORD wSizeSinglePkg; //设置的分包大小
	WORD nPicPkg; //总包数

	int nCountValidPkg[1000] = { 0 }; //第N包有效数据大小
	unsigned char cPicDest[1000][4196];  //[PkgNo][Data]保存收到的所有数据

	//CommSetting_T tCommSetting;
};

extern CZXiGateApp theApp;