// CommMFC.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "ZXiGate.h"
#include "ZXiGateDlg.h"

#include <mysql.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CZXiGateApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCommMFCApp ����

CZXiGateApp::CZXiGateApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
	nTypeTranslateData = TRANSTYOE_MYSQL;
}


// Ψһ��һ�� CCommMFCApp ����

CZXiGateApp theApp;


// CCommMFCApp ��ʼ��

BOOL CZXiGateApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("ZXiMemoryStick"));

	CZXiGateDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

int CZXiGateApp::DataLoading(CString strSQL)
{
	int nRet = 0;
	MYSQL_RES* res;

	mysql_query(&mysql, strSQL);
	//if (mysql_query(&mysql, strSQL)) { //ִ��insert������������
	//	nRet =  0;
	//}
	//else {
	//	nRet = -1;
	//	return nRet;
	//}

	//��ȡ�����
	res = mysql_store_result(&mysql); //����ֵ��insert��Ч,���Ƿ���NULL

//	if (res == NULL) {
//		nRet = -1;
//		mysql_free_result(res);
//		return nRet;
//	}

//	int rows = mysql_num_rows(res);
//	mysql_free_result(res); //�ͷŽ����

//	if (rows == 0) //ͨ���ж����õķ��������ж��Ƿ��в�ѯ��������
//	{
//		nRet = -1;
//	}
	if (mysql_errno(&mysql) == 0) nRet = 0;
	else nRet = -1;

	return nRet;
}

void CZXiGateApp::DisConnectDatabase(void)
{
	mysql_close(&mysql);
}

int CZXiGateApp::ConnectDatabase(void)
{
//	MYSQL_RES* res; //������ṹ��
//	MYSQL_ROW row;  //char** ��ά���飬��ż�¼

	int nRet = 0;

	//��ʼ�����ݿ�
	mysql_init(&mysql);

	//���ñ��뷽ʽ
	mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, "gbk");

	if (mysql_real_connect(&mysql, strHostDatebase, strUserDatabase, strPasswdDatabase, strDbDatabase, 3306, NULL, 0) == NULL) {
		TRACE(mysql_error(&mysql));
		nRet = -1;
	}
	else {
		TRACE(mysql_error(&mysql));
		nRet = 0;
	}

	return nRet;
}

/*
	database:	sensor_igate
	table:		sensor_node_rt
	struction:
	id				int			id auto
	node_id			varchar		node number
	temp			varchar		NTC Temperature
	tempc			varchar		Cell Temperature
	db_datetime		datetime	update dbase time
	node_datetime	datetime	sensor node time
*/
//���ݿⷽʽ����
void CZXiGateApp::RunConnectMySQL(CString strSQL)
{
	MYSQL mysql;    //���ݿ�ṹ��
	MYSQL_RES* res; //������ṹ��
	MYSQL_ROW row;  //char** ��ά���飬��ż�¼

	CString str;
	int nRet;

	//��ʼ�����ݿ�
	mysql_init(&mysql);

	//���ñ��뷽ʽ
	mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, "gbk");

	if (mysql_real_connect(&mysql, "rm-bp159an86cl84v71p6o.mysql.rds.aliyuncs.com", "root", "root", "sensor_igate", 3306, NULL, 0) == NULL) {

		str.Format("%s", mysql_error(&mysql));
		//2022-8-14,д����־
		//MessageBox(NULL, str, "���ݿ����Ӵ���", MB_OK);
	}
	else {
		TRACE(mysql_error(&mysql));
	}

	//��ѯ����
	//CString strSQL;

	//strSQL = "SELECT * from tb_sensor_node";
	nRet = mysql_query(&mysql, strSQL);
	TRACE("%d\n", nRet);

	//��ȡ�����
	res = mysql_store_result(&mysql);

	sensor_node = new CSensorNode();
	/*
	//ȡ���ֶθ���������
	int fieldcount = mysql_num_fields(res);

	MYSQL_FIELD* field = NULL;
	for (unsigned int i = 0; i < fieldcount; i++) {
		field = mysql_fetch_field_direct(res, i);
		TRACE(field->name);
		TRACE("\r\n");
	}

	//ȡ�ü�¼��
	node_rowcount = mysql_num_rows(res);

	int niRow = 0;


	while (row = mysql_fetch_row(res))
	{
		//sensor_node[niRow] = new CSensorNode();
		TRACE("\r\n");
		sensor_node[niRow].id = atoi(row[0]);
		sensor_node[niRow].node_id = row[1];
		sensor_node[niRow].temp = atof(row[2]);
		sensor_node[niRow].tempc = atof(row[3]);
		sensor_node[niRow].db_datetime = row[4];
		sensor_node[niRow].node_datetime = row[5];
		//TRACE(row[0]);		TRACE("\r\n");
		//TRACE(row[1]);		TRACE("\r\n");
		//TRACE(row[2]);		TRACE("\r\n");
		//TRACE(row[3]);		TRACE("\r\n");
		//TRACE(row[4]);		TRACE("\r\n");
		//TRACE(row[5]);		TRACE("\r\n");
		niRow++;
	}
	
	mysql_free_result(res); //�ͷŽ����
	mysql_close(&mysql); //�ر����ݿ�
	*/
}