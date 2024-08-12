// CommMFC.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

#include <mysql.h>

using namespace std;

//�������ڵ���
class CSensorNode {
public:
	int id;					//�Զ�ID
	CString node_id;		//�ڵ�ID
	float	temp;			//�ڵ��¶�
	float	tempc;			//����¶�
	CString	db_datetime;	//���ʱ��
	CString node_datetime;	//�ڵ���������ʱ��
};

//���ݶ�д��ʽ
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

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()

public:
	CString strRetCurl;
	//static size_t write_memory_callback(void* contents, size_t size, size_t nmemb, void* userp);
	//int RunCurl();

	uint64_t node_rowcount; //��¼����
	void RunConnectMySQL(CString strSQL); //�������ݿ⡢������⣬������ͣʹ��
	MYSQL mysql;
	int ConnectDatabase(void);		//�������ݿ�
	void DisConnectDatabase(void);	//�Ͽ����ݿ�

	int DataLoading(CString strSQL); //����д�����ݿ�

	CSensorNode* sensor_node = new CSensorNode[1024];
	CString strHostDatebase; //���ݿ��ַ
	CString strUserDatabase; //���ݿ��¼��
	CString strPasswdDatabase; //���ݿ��¼����
	CString strDbDatabase; //���ݿ���
	//MYSQL strErrDataase; //�������ݿⷵ�صĴ�����

	int nTypeTranslateData; //DTU��ƽ̨�����ݴ��䷽ʽ

	WORD wCamCmd; //���������֡��0x5A23(Z#)-��ȡ�汾, 0x4823(H#)-������Ƭ,   0x4523(E#)-��ȡָ��������, 
								//0x4423(D#)-����ID,   0x4923(I#)-���ò�����, 0x5123(Q#)-����ѹ���ʡ�
	WORD wSizeSinglePkg; //���õķְ���С
	WORD nPicPkg; //�ܰ���

	int nCountValidPkg[1000] = { 0 }; //��N����Ч���ݴ�С
	unsigned char cPicDest[1000][4196];  //[PkgNo][Data]�����յ�����������

	//CommSetting_T tCommSetting;
};

extern CZXiGateApp theApp;